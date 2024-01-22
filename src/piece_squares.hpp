//
// Created by christian on 1/22/24.
//

#ifndef CHESS_WEEKEND_PIECE_SQUARES_HPP
#define CHESS_WEEKEND_PIECE_SQUARES_HPP

using namespace lczero;

constexpr BitBoard piece_square_knight_bits[] = {
        0x1866245a1800,
        0x42ff425a5a42ff42,
        0x3c42bdbdbdbd423c,
        0x3c7e7e7e7e3c00,
        0x0,
};
constexpr BitBoard piece_square_pawn_bits[] = {
        0xdb00c38100,
        0xff0024dbff9999ff,
        0xffe73c18007e00,
        0xff00e7ffffffe7ff,
        0xff180000000000,
};
constexpr BitBoard piece_square_bishop_bits[] = {
        0x246600004200,
        0x817e666642007e81,
        0x810018183c7e0081,
        0x7effffffffffff7e,
        0x0,
};
constexpr BitBoard piece_square_rook_bits[] = {
        0x0081818181818118,
        0xff817e7e7e7e7eff,
        0x7e000000000000,
        0xffffffffffffffff,
        0x0,
};
constexpr BitBoard piece_square_queen_bits[] = {
        0x18003cbd3d7c2018,
        0x817e7e7efe7e7e81,
        0x8100000000000081,
        0x7effffffffffff7e,
        0x0,
};
constexpr BitBoard piece_square_king_middle_game_bits[] = {
        0x0,
        0x66666666997eff99,
        0x81818181e77ec3a5,
        0x81ffbd,
        0x42,
};
constexpr BitBoard piece_square_king_end_game_bits[] = {
        0x0,
        0x5a5a241818243c00,
        0x3cc3a58181a5c37e,
        0x3c664242663c00,
        0x183c3c180000,
};

inline int32_t score_position_with_piece_squares(const Position& position)
{
  auto our_board = position.GetBoard();
  auto their_board = position.GetThemBoard();



  auto our_pawns = our_board.pawns()&our_board.ours();
  auto their_pawns = their_board.pawns()&their_board.ours();
  auto our_knights = our_board.knights()&our_board.ours();
  auto their_knights = their_board.knights()&their_board.ours();
  auto our_bishops = our_board.bishops()&our_board.ours();
  auto their_bishops = their_board.bishops()&their_board.ours();
  auto our_rooks = our_board.rooks()&our_board.ours();
  auto their_rooks = their_board.rooks()&their_board.ours();
  auto our_queens = our_board.queens()&our_board.ours();
  auto their_queens = their_board.queens()&their_board.ours();
  auto our_kings = our_board.kings()&our_board.ours();
  auto their_kings = their_board.kings()&their_board.ours();

  BitBoard our_0_bits;
  BitBoard our_1_bits;
  BitBoard our_2_bits;
  BitBoard our_3_bits;
  BitBoard our_4_bits;

  our_0_bits = our_0_bits | (our_pawns&piece_square_pawn_bits[0]);
  our_1_bits = our_1_bits | (our_pawns&piece_square_pawn_bits[1]);
  our_2_bits = our_2_bits | (our_pawns&piece_square_pawn_bits[2]);
  our_3_bits = our_3_bits | (our_pawns&piece_square_pawn_bits[3]);
  our_4_bits = our_4_bits | (our_pawns&piece_square_pawn_bits[4]);

  our_0_bits = our_0_bits | (our_knights&piece_square_knight_bits[0]);
  our_1_bits = our_1_bits | (our_knights&piece_square_knight_bits[1]);
  our_2_bits = our_2_bits | (our_knights&piece_square_knight_bits[2]);
  our_3_bits = our_3_bits | (our_knights&piece_square_knight_bits[3]);
  our_4_bits = our_4_bits | (our_knights&piece_square_knight_bits[4]);

  our_0_bits = our_0_bits | (our_bishops&piece_square_bishop_bits[0]);
  our_1_bits = our_1_bits | (our_bishops&piece_square_bishop_bits[1]);
  our_2_bits = our_2_bits | (our_bishops&piece_square_bishop_bits[2]);
  our_3_bits = our_3_bits | (our_bishops&piece_square_bishop_bits[3]);
  our_4_bits = our_4_bits | (our_bishops&piece_square_bishop_bits[4]);

  our_0_bits = our_0_bits | (our_rooks&piece_square_rook_bits[0]);
  our_1_bits = our_1_bits | (our_rooks&piece_square_rook_bits[1]);
  our_2_bits = our_2_bits | (our_rooks&piece_square_rook_bits[2]);
  our_3_bits = our_3_bits | (our_rooks&piece_square_rook_bits[3]);
  our_4_bits = our_4_bits | (our_rooks&piece_square_rook_bits[4]);

  our_0_bits = our_0_bits | (our_queens&piece_square_queen_bits[0]);
  our_1_bits = our_1_bits | (our_queens&piece_square_queen_bits[1]);
  our_2_bits = our_2_bits | (our_queens&piece_square_queen_bits[2]);
  our_3_bits = our_3_bits | (our_queens&piece_square_queen_bits[3]);
  our_4_bits = our_4_bits | (our_queens&piece_square_queen_bits[4]);

  our_0_bits = our_0_bits | (our_kings&piece_square_king_middle_game_bits[0]);
  our_1_bits = our_1_bits | (our_kings&piece_square_king_middle_game_bits[1]);
  our_2_bits = our_2_bits | (our_kings&piece_square_king_middle_game_bits[2]);
  our_3_bits = our_3_bits | (our_kings&piece_square_king_middle_game_bits[3]);
  our_4_bits = our_4_bits | (our_kings&piece_square_king_middle_game_bits[4]);

  BitBoard their_0_bits;
  BitBoard their_1_bits;
  BitBoard their_2_bits;
  BitBoard their_3_bits;
  BitBoard their_4_bits;

  their_0_bits = their_0_bits | (their_pawns&piece_square_pawn_bits[0]);
  their_1_bits = their_1_bits | (their_pawns&piece_square_pawn_bits[1]);
  their_2_bits = their_2_bits | (their_pawns&piece_square_pawn_bits[2]);
  their_3_bits = their_3_bits | (their_pawns&piece_square_pawn_bits[3]);
  their_4_bits = their_4_bits | (their_pawns&piece_square_pawn_bits[4]);

  their_0_bits = their_0_bits | (their_knights&piece_square_knight_bits[0]);
  their_1_bits = their_1_bits | (their_knights&piece_square_knight_bits[1]);
  their_2_bits = their_2_bits | (their_knights&piece_square_knight_bits[2]);
  their_3_bits = their_3_bits | (their_knights&piece_square_knight_bits[3]);
  their_4_bits = their_4_bits | (their_knights&piece_square_knight_bits[4]);

  their_0_bits = their_0_bits | (their_bishops&piece_square_bishop_bits[0]);
  their_1_bits = their_1_bits | (their_bishops&piece_square_bishop_bits[1]);
  their_2_bits = their_2_bits | (their_bishops&piece_square_bishop_bits[2]);
  their_3_bits = their_3_bits | (their_bishops&piece_square_bishop_bits[3]);
  their_4_bits = their_4_bits | (their_bishops&piece_square_bishop_bits[4]);

  their_0_bits = their_0_bits | (their_rooks&piece_square_rook_bits[0]);
  their_1_bits = their_1_bits | (their_rooks&piece_square_rook_bits[1]);
  their_2_bits = their_2_bits | (their_rooks&piece_square_rook_bits[2]);
  their_3_bits = their_3_bits | (their_rooks&piece_square_rook_bits[3]);
  their_4_bits = their_4_bits | (their_rooks&piece_square_rook_bits[4]);

  their_0_bits = their_0_bits | (their_queens&piece_square_queen_bits[0]);
  their_1_bits = their_1_bits | (their_queens&piece_square_queen_bits[1]);
  their_2_bits = their_2_bits | (their_queens&piece_square_queen_bits[2]);
  their_3_bits = their_3_bits | (their_queens&piece_square_queen_bits[3]);
  their_4_bits = their_4_bits | (their_queens&piece_square_queen_bits[4]);

  their_0_bits = their_0_bits | (their_kings&piece_square_king_middle_game_bits[0]);
  their_1_bits = their_1_bits | (their_kings&piece_square_king_middle_game_bits[1]);
  their_2_bits = their_2_bits | (their_kings&piece_square_king_middle_game_bits[2]);
  their_3_bits = their_3_bits | (their_kings&piece_square_king_middle_game_bits[3]);
  their_4_bits = their_4_bits | (their_kings&piece_square_king_middle_game_bits[4]);

  int32_t bits_0 = our_0_bits.count() - their_0_bits.count();
  int32_t bits_1 = our_1_bits.count() - their_1_bits.count();
  int32_t bits_2 = our_2_bits.count() - their_2_bits.count();
  int32_t bits_3 = our_3_bits.count() - their_3_bits.count();
  int32_t bits_4 = our_4_bits.count() - their_4_bits.count();

  int32_t total = bits_4*(1<<4) + bits_3*(1<<3) + bits_2*(1<<2) + bits_1*(1<<1) + bits_0;
  // Account for 5x scaler
  total = total*5;
  // Account for -50 per piece offset
  int32_t num_pieces = (our_board.ours().count()) - (their_board.theirs().count());
  total += num_pieces*-50;
  return total;
}
#endif //CHESS_WEEKEND_PIECE_SQUARES_HPP
