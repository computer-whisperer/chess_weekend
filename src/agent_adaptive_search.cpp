//
// Created by christian on 1/20/24.
//
#include <iostream>
#include <cstring>
#include <chrono>
#include "board.h"
#include "uciloop.h"
#include "christian_utils.h"
#include "piece_squares.hpp"

using namespace lczero;

BitBoard rank_8(0xFF00000000000000);
BitBoard rank_7(0x00FF000000000000);
BitBoard rank_6(0x0000FF0000000000);
BitBoard rank_5(0x000000FF00000000);
BitBoard rank_4(0x00000000FF000000);
BitBoard rank_3(0x0000000000FF0000);
BitBoard rank_2(0x000000000000FF00);
BitBoard rank_1(0x00000000000000FF);

bool do_randomization = false;

int32_t score_end_states(const Position& position, MoveList& legal_moves)
{
  auto& board = position.GetBoard();
  bool is_draw = false;
  if (legal_moves.empty()) {
    if (board.IsUnderCheck()) {
      // Checkmate.
      // We lost
      return -10000 + position.GetGamePly()*10;
    }
    // Stalemate.
    is_draw = true;
  }

  if (!board.HasMatingMaterial()) is_draw = true;
  if (position.GetRule50Ply() >= 100) is_draw = true;
  if (position.GetRepetitions() >= 2) is_draw = true;

  if (is_draw) return -1000;
  return 0;
}

constexpr int32_t ABS_MIN_SCORE = -1000000000;
constexpr int32_t ABS_MAX_SCORE =  1000000000;

class TreeNode
{
public:
  int64_t budget_used = 0;
  MoveList legal_moves;
  std::vector<TreeNode> children;
  int32_t best_score = ABS_MIN_SCORE;
  int32_t own_score = ABS_MIN_SCORE;
  uint32_t best_move_idx = 0;

  TreeNode extractChild(uint32_t idx)
  {
    TreeNode child;
    child.budget_used = children[idx].budget_used;
    child.best_score = children[idx].best_score;
    child.own_score = children[idx].own_score;
    child.best_move_idx = children[idx].best_move_idx;
    child.legal_moves.swap(children[idx].legal_moves);
    child.children.swap(children[idx].children);
    return child;
  }
};

void budgeted_search(PositionHistory& position_history, int64_t allowed_budget, int64_t& budget_used, int32_t alpha, int32_t beta, TreeNode& node)
{
  budget_used = 0;

  auto& position = position_history.Last();
  auto& board = position.GetBoard();
  if (node.budget_used == 0)
  {
    node.legal_moves = board.GenerateLegalMoves();
    node.children.reserve(node.legal_moves.size());
    for (uint32_t i = 0; i < node.legal_moves.size(); i++)
    {
      node.children.emplace_back();
    }
    budget_used++;
    node.budget_used++;
  }
  auto num_moves = node.legal_moves.size();

  // Handle end states
  auto end_score = score_end_states(position, node.legal_moves);
  if (end_score != 0)
  {
    node.own_score = end_score;
    node.best_score = node.own_score;
    return;
  }

  // Basic scoring
  if (node.own_score == ABS_MIN_SCORE)
  {
    // Basic material scoring
    node.own_score = 0;
    node.own_score += ((board.queens()&board.ours()).count() - (board.queens()&board.theirs()).count())*900;
    node.own_score += ((board.rooks()&board.ours()).count() - (board.rooks()&board.theirs()).count())*500;
    node.own_score += ((board.bishops()&board.ours()).count() - (board.bishops()&board.theirs()).count())*300;
    node.own_score += ((board.knights()&board.ours()).count() - (board.knights()&board.theirs()).count())*300;
    node.own_score += ((board.pawns()&board.ours()).count() - (board.pawns()&board.theirs()).count())*100;

    // Calculate legal moves
    //node.own_score += (int32_t)(node.legal_moves.size() - position.GetThemBoard().GenerateLegalMoves().size())*10;

    node.own_score += score_position_with_piece_squares(position);

    /*
    // Board progression scoring
    node.own_score += ((board.ours()&rank_3).count() - (board.theirs()&rank_6).count())*5;
    node.own_score += ((board.ours()&rank_4).count() - (board.theirs()&rank_5).count())*10;
    node.own_score += ((board.ours()&rank_5).count() - (board.theirs()&rank_4).count())*15;
    node.own_score += ((board.ours()&rank_6).count() - (board.theirs()&rank_3).count())*20;
    node.own_score += ((board.ours()&rank_7).count() - (board.theirs()&rank_2).count())*20;*/

    if (do_randomization)
    {
      node.own_score += fast_rand()%10;
    }
  }

  if (node.best_score == ABS_MIN_SCORE)
  {
    node.best_score = node.own_score;
  }

  if (allowed_budget - budget_used <= 1)
  {
    return;
  }

  // Allocate search budget
  float mean_score = 0;
  int32_t min_score = ABS_MAX_SCORE;
  int32_t max_score = ABS_MIN_SCORE;

  // Run light scans if needed, and get mean, min, and max scores
  for (uint32_t i = 0; i < num_moves; ++i)
  {
    if (node.children[i].budget_used == 0)
    {
      // Must light scan this node
      position_history.Append(node.legal_moves[i]);
      int64_t local_budget_used;
      budgeted_search(position_history, (allowed_budget/200) + 1, local_budget_used, ABS_MIN_SCORE, ABS_MAX_SCORE, node.children[i]);
      budget_used += local_budget_used;
      node.budget_used += local_budget_used;
      position_history.Pop();
    }
    mean_score += (float)-node.children[i].best_score;
    min_score = std::min(min_score, -node.children[i].best_score);
    max_score = std::max(max_score, -node.children[i].best_score);
  }
  mean_score /= (float)num_moves;

  // If we have extreme low values, bound
  min_score = std::max(min_score, (int32_t)mean_score - 400);

  // Calculate scores for each child
  int32_t total_score = 0;
  int32_t move_scores[num_moves];
  for (uint32_t i = 0; i < num_moves; ++i)
  {
    move_scores[i] = std::max((-node.children[i].best_score) - min_score, 0) + 100;
    total_score += move_scores[i];
  }

  // Decide how much budget to use on each score point
  float ratio = (float)(allowed_budget - budget_used) / (float)total_score;

  uint32_t new_best_move_idx = 0;
  int32_t new_best_score = ABS_MIN_SCORE;


  // Iterate based on the search budget
  bool has_seen_move[num_moves];
  memset(has_seen_move, 0, sizeof(has_seen_move));

  while(true)
  {
    // Iterate w/ best move first
    int32_t next_idx = -1;
    int32_t next_idx_score = ABS_MIN_SCORE;
    for (int32_t i = 0; i < num_moves; i++)
    {
      if (!has_seen_move[i] && move_scores[i] > next_idx_score)
      {
        next_idx = i;
        next_idx_score = move_scores[i];
      }
    }
    if (next_idx < 0)
    {
      break;
    }
    has_seen_move[next_idx] = true;
    if (move_scores[next_idx] > 0)
    {
      position_history.Append(node.legal_moves[next_idx]);
      auto inner_budget = (int32_t)(ratio*(float)move_scores[next_idx]);
      int64_t inner_budget_used = 0;
      budgeted_search(position_history, inner_budget, inner_budget_used, -beta, -alpha, node.children[next_idx]);
      budget_used += inner_budget_used;
      node.budget_used += inner_budget_used;
      auto inner_score = -node.children[next_idx].best_score;
      if (inner_score > beta)
      {
        // Dead end
        new_best_score = inner_score;
        new_best_move_idx = next_idx;
        position_history.Pop();
        break;
      }

      if (alpha < inner_score)
      {
        alpha = inner_score;
      }

      if (inner_score > new_best_score)
      {
        new_best_score = inner_score;
        new_best_move_idx = next_idx;
      }
      position_history.Pop();
    }
  }

  if (new_best_score != ABS_MIN_SCORE)
  {
    node.best_move_idx = new_best_move_idx;
    node.best_score = new_best_score;
  }
}


void getBestLine(TreeNode& node, MoveList& best_line)
{
  if (node.legal_moves.empty())
  {
    // Base case
    return;
  }
  best_line.push_back(node.legal_moves[node.best_move_idx]);
  getBestLine(node.children[node.best_move_idx], best_line);
}




class CustomUCILoop : public UciLoop {
  PositionHistory root_position_history;
  TreeNode root_node;

  void dump_info(TreeNode& node, PositionHistory& position_history)
  {
    MoveList best_line;
    getBestLine(node, best_line);
    bool black_to_move = position_history.IsBlackToMove();
    ThinkingInfo info;
    info.depth = 0;
    for (auto& move : best_line)
    {
      auto m_move = move;
      if (black_to_move)
      {

        m_move.Mirror();
      }
      info.pv.push_back(m_move);
      black_to_move = !black_to_move;
      info.depth++;
    }
    info.multipv = 1;
    info.nodes = node.budget_used;
    info.score = node.best_score;
    std::vector<ThinkingInfo> infos{info};
    SendInfo(infos);
  }

  void thinkForTime(std::chrono::duration<double> allowed_seconds, PositionHistory& position_history, TreeNode& node)
  {
    const auto start{std::chrono::steady_clock::now()};
    int64_t budget_used = 0;
    while ((std::chrono::steady_clock::now() - start) < allowed_seconds)
    {
      int64_t total_budget = 10000;
      int64_t local_budget_used = 0;
      budgeted_search(position_history, total_budget, local_budget_used, ABS_MIN_SCORE, ABS_MAX_SCORE, node);
      budget_used += local_budget_used;
    }
    const std::chrono::duration<double> elapsed_seconds{std::chrono::steady_clock::now() - start};
    std::cout << "Time: " << elapsed_seconds << std::endl;
    std::cout << "Budget: "<< budget_used << std::endl;
    std::cout << "Best line:";
    MoveList best_line;
    getBestLine(node, best_line);
    bool black_to_move = position_history.IsBlackToMove();
    for (auto& move : best_line)
    {
      if (black_to_move)
      {
        auto m_move = move;
        m_move.Mirror();
        std::cout << " " << m_move.as_string();
      }
      else
      {
        std::cout << " " << move.as_string();
      }
      black_to_move = !black_to_move;
    }
    std::cout << std::endl;

  }

  void thinkForBudget(int64_t budget, PositionHistory& position_history, TreeNode& node)
  {
    const auto start{std::chrono::steady_clock::now()};
    int64_t budget_used = 0;
    int64_t iteration_budget = 10000;
    while (budget_used < budget)
    {
      bool last_iteration = false;
      if (budget - budget_used < iteration_budget)
      {
        iteration_budget = budget - budget_used;
        last_iteration = true;
      }
      int64_t local_budget_used = 0;
      budgeted_search(position_history, iteration_budget, local_budget_used, ABS_MIN_SCORE, ABS_MAX_SCORE, node);
      budget_used += local_budget_used;
      dump_info(node, position_history);
      if (last_iteration)
      {
        break;
      }
      if (local_budget_used == 0)
      {
        if ((iteration_budget + budget_used) < (budget-10000))
        {
          // Need much more budget!
          //std::cout << "Did nothing last iteration!" << std::endl;
          iteration_budget = iteration_budget * 10;
        }
        else
        {
          // Actually ran out of stuff to do
          break;
        }
      }

      iteration_budget = (iteration_budget * 3)/2;
    }
    const std::chrono::duration<double> elapsed_seconds{std::chrono::steady_clock::now() - start};
    //std::cout << "Time: " << elapsed_seconds << std::endl;
    //std::cout << "Budget: "<< budget_used << std::endl;
    dump_info(node, position_history);
  }


  void SendId() override {
    SendResponse("id name WhisperChess 1.1.0");
    SendResponse("id author computer-whisperer");
  }


  void CmdUci() override {
    SendId();
    SendResponse("uciok");
  }
  void CmdIsReady() override {SendResponse("readyok");}
  void CmdUciNewGame() override {
    if (do_randomization)
    {
      long r = random();
      seed_fast_rand((int)r%1000);
    }
    root_node = TreeNode();
    root_position_history.Reset(ChessBoard::kStartposBoard, 0, 0);
    //thinkFor(std::chrono::seconds(4), position_history, meta_tree);
  }
  void CmdSetOption(const std::string& /*name*/,
                    const std::string& /*value*/,
                    const std::string& /*context*/) override {
    SendResponse("setoption ok");
  }
  void CmdPosition(const std::string& position,
                   const std::vector<std::string>& moves) override {
    if (position.empty())
    {
      PositionHistory new_position_history;
      new_position_history.Reset(ChessBoard::kStartposBoard, 0, 0);
      // Assume start position is the same

      bool reset_tree = moves.empty();
      reset_tree = true;
      for (int32_t move_idx = 0; move_idx < moves.size(); move_idx++)
      {
        Move real_move = moves[move_idx];
        if (new_position_history.IsBlackToMove())
        {
          real_move.Mirror();
        }
        new_position_history.Append(real_move);

        if (!reset_tree)
        {
          if (root_position_history.GetLength() > move_idx+1)
          {
            // Check that the tree matches
            if (root_position_history.GetPositionAt(move_idx+1).GetBoard() != new_position_history.GetPositionAt(move_idx+1).GetBoard())
            {
              reset_tree = true;
            }
          }
          else
          {
            // Migrate tree forwards
            if (root_node.legal_moves.empty())
            {
              reset_tree = true;
            }
            else
            {
              uint32_t legal_move_idx = 0;
              uint32_t num_legal_moves = root_node.legal_moves.size();
              for (; legal_move_idx < num_legal_moves; legal_move_idx++)
              {
                if (root_node.legal_moves[legal_move_idx] == real_move)
                {
                  root_node = root_node.extractChild(legal_move_idx);
                  break;
                }
              }
              if (legal_move_idx == num_legal_moves)
              {
                // Couldn't find move
                reset_tree = true;
              }
            }

          }
        }
      }
      root_position_history = new_position_history;
      if (reset_tree)
      {
        root_node = TreeNode();
      }
    }
  }

  void CmdGo(const GoParams& /*params*/) override {
    //thinkForTime(std::chrono::seconds(5), position_history, meta_tree);
    thinkForBudget(20000000, root_position_history, root_node);
    auto move = root_node.legal_moves[root_node.best_move_idx];
    if (root_position_history.IsBlackToMove())
    {
      move.Mirror();
    }
    //std::cout << "Move (after flip): " << move.as_string() << std::endl << std::endl;
    SendBestMove(move);
  }

};

int main() {
  InitializeMagicBitboards();
  /*
  ChessBoard board(ChessBoard::kStartposFen);
  std::cout << board.DebugString();

  auto list = board.GeneratePseudolegalMoves();
  std::cout  << std::endl << "Have " << list.size() << " possible moves." << std::endl;
  auto move = list[0];
  std::cout << "Move: " << move.as_string() << std::endl << std::endl;

  board.ApplyMove(move);

  std::cout << board.DebugString();*/

  CustomUCILoop uci_loop;
  uci_loop.RunLoop();
  return 0;
}
