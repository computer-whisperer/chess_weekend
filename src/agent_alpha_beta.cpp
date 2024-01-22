#include <iostream>
#include "board.h"
#include "uciloop.h"
#include "christian_utils.h"

using namespace lczero;

BitBoard rank_8(0xFF00000000000000);
BitBoard rank_7(0x00FF000000000000);
BitBoard rank_6(0x0000FF0000000000);
BitBoard rank_5(0x000000FF00000000);
BitBoard rank_4(0x00000000FF000000);
BitBoard rank_3(0x0000000000FF0000);
BitBoard rank_2(0x000000000000FF00);
BitBoard rank_1(0x00000000000000FF);

int basicChessScore(ChessBoard board, int turn_num)
{
  int score = 0;
  if (board.GenerateLegalMoves().empty()) {
    if (board.IsUnderCheck()) {
      // Checkmate.
      return -50000 + turn_num*10;
    }
  }

  // Basic material scoring
  score += ((board.queens()&board.ours()).count() - (board.queens()&board.theirs()).count())*900;
  score += ((board.rooks()&board.ours()).count() - (board.rooks()&board.theirs()).count())*500;
  score += ((board.bishops()&board.ours()).count() - (board.bishops()&board.theirs()).count())*300;
  score += ((board.knights()&board.ours()).count() - (board.knights()&board.theirs()).count())*300;
  score += ((board.pawns()&board.ours()).count() - (board.pawns()&board.theirs()).count())*100;

  // Board progression scoring
  score += ((board.ours()&rank_3).count() - (board.theirs()&rank_6).count())*5;
  score += ((board.ours()&rank_4).count() - (board.theirs()&rank_5).count())*10;
  score += ((board.ours()&rank_5).count() - (board.theirs()&rank_4).count())*15;
  score += ((board.ours()&rank_6).count() - (board.theirs()&rank_3).count())*20;
  score += ((board.ours()&rank_7).count() - (board.theirs()&rank_2).count())*20;

  // Incentivise development
  auto our_rank1_pieces = board.ours()&rank_1;
  auto their_rank1_pieces = board.theirs()&rank_8;
  score += ((our_rank1_pieces&board.queens()).count() - (their_rank1_pieces&board.queens()).count())*-10;
  score += ((our_rank1_pieces&board.rooks()).count() - (their_rank1_pieces&board.rooks()).count())*-20;
  score += ((our_rank1_pieces&board.bishops()).count() - (their_rank1_pieces&board.bishops()).count())*-20;
  score += ((our_rank1_pieces&board.knights()).count() - (their_rank1_pieces&board.knights()).count())*-20;

  return score;
}

int findBestMove_inner(ChessBoard board, int turn_num, int depth, Move* move_out, int alpha, int beta)
{
  auto legal_moves = board.GenerateLegalMoves();
  if (legal_moves.empty()) {
    if (board.IsUnderCheck()) {
      // Checkmate.
      return -50000 + turn_num*10;
    }
  }

  if (depth == 0)
  {
    Move bestMove = legal_moves[0];
    int bestScore = -100000000;

    for (auto move : legal_moves)
    {
      auto new_board = board;
      new_board.ApplyMove(move);
      int score = basicChessScore(new_board, turn_num);
      if (score > bestScore)
      {
        bestMove = move;
        bestScore = score;
      }
    }
    if (move_out)
    {
      *move_out = bestMove;
    }
    return bestScore;
  }

  Move bestMove;
  int bestScore = -100000000;

  for (auto move : legal_moves)
  {
    auto new_board = board;
    new_board.ApplyMove(move);
    new_board.Mirror();
    int score;
    score = -findBestMove_inner(new_board, turn_num+1, depth-1, nullptr, -beta, -alpha);

    if (score > beta)
    {
      // Dead end
      bestMove = move;
      bestScore = score;
      break;
    }

    if (alpha < score)
    {
      alpha = score;
    }

    if (score >= bestScore)
    {
      bestMove = move;
      bestScore = score;
    }
  }

  if (move_out)
  {
    *move_out = bestMove;
  }
  return bestScore;

}

Move findBestMove(ChessBoard board, int turn_num, int depth)
{
  Move out;
  findBestMove_inner(board, turn_num, depth, &out, -1000000000, 1000000000);
  return out;
}


class CustomUCILoop : public UciLoop {
  ChessBoard current_board;
  int turn_num = 0;

  void CmdUci() override {
    SendId();
    SendResponse("uciok");
  }
  void SendId() override {
    SendResponse("id name WhisperChess 0.3.0");
    SendResponse("id author computer-whisperer");
  }
  void CmdIsReady() override {SendResponse("readyok");}
  void CmdUciNewGame() override {}
  void CmdSetOption(const std::string& /*name*/,
                    const std::string& /*value*/,
                    const std::string& /*context*/) override {
    SendResponse("setoption ok");
  }
  void CmdPosition(const std::string& position,
                   const std::vector<std::string>& moves) override {
    if (position.empty())
    {
      current_board = ChessBoard::kStartposBoard;
      for (const auto& move : moves)
      {
        Move real_move(move);
        if (current_board.flipped())
        {
          real_move.Mirror();
        }
        current_board.ApplyMove(real_move);
        current_board.Mirror();
        turn_num++;
      }
    }
    else
    {
      int n_moves;
      current_board.SetFromFen(position, nullptr, &n_moves);
      turn_num = n_moves;
    }

  }

  void CmdGo(const GoParams& /*params*/) override {
    auto move = findBestMove(current_board, turn_num, 4);
    //auto move = findBestMove(current_board, turn_num, 1, nullptr, -1000000000, 1000000000);
    if (current_board.flipped())
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
