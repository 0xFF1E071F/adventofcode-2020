#include <bitset>
#include <cmath>
#include <iostream>
#include <iterator>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using bs = std::bitset<32>;

typedef uint16_t tileid_t;
typedef size_t tilenum_t;

struct tile_t {
  tileid_t id;
  int side;
  std::string data;

  void compute_edges();

  void flip_top_bottom();
  void flip_left_right();
  void rotate_cw();

  uint16_t top = 0, bottom = 0, left = 0, right = 0;
};

std::ostream &operator<<(std::ostream &out, const tile_t &tile) {
  for (auto l = 0; l < tile.side; ++l)
    out << tile.data.substr(l * tile.side, tile.side) << std::endl;

  return out;
}

void tile_t::compute_edges() {
  bs line(data, 0, side, '.', '#');
  top = (uint16_t)line.to_ulong();

  line = bs(data, side * (side - 1), side, '.', '#');
  bottom = (uint16_t)line.to_ulong();

  line.reset();
  for (auto i = 0; i < side; ++i)
    line[side - i - 1] = data[i * side] == '#' ? true : false;
  left = (uint16_t)line.to_ulong();

  line.reset();
  for (auto i = 0; i < side; ++i)
    line[side - i - 1] = data[(i + 1) * side - 1] == '#' ? true : false;
  right = (uint16_t)line.to_ulong();
}

void tile_t::flip_top_bottom() {
  std::string ndata{};

  for (auto i = side - 1; i >= 0; --i)
    ndata += data.substr(i * side, side);

  data = std::move(ndata);

  compute_edges();
}

void tile_t::flip_left_right() {
  std::string ndata{};

  for (auto i = 0; i < side; ++i) {
    const std::string line = data.substr(i * side, side);
    std::copy(line.crbegin(), line.crbegin() + side, std::back_inserter(ndata));
  }

  data = std::move(ndata);

  compute_edges();
}

void tile_t::rotate_cw() {
  std::string ndata = data;

  for (auto row = 0; row < side; ++row)
    for (auto col = 0; col < side; ++col)
      // x, y = y, side - x
      ndata[row * side + col] = data[(side - col - 1) * side + row];

  data = std::move(ndata);

  compute_edges();
}

std::vector<tile_t> tiles;
size_t tiling_size;
size_t tiling_side;

std::unordered_map<uint16_t, std::vector<tilenum_t>> tile_by_left{};
std::unordered_map<uint16_t, std::vector<tilenum_t>> tile_by_top{};

std::unordered_map<uint32_t, std::vector<tilenum_t>> tile_by_lefttop{};

void add(const tile_t &tile) {
  const tilenum_t tile_num = tiles.size();
  tiles.push_back(tile);

  const auto left = tile.left;

  if (auto v = tile_by_left.find(left); v != tile_by_left.end()) {
    (*v).second.push_back(tile_num);
  } else {
    tile_by_left.emplace(left, std::vector{tile_num});
  }

  const auto top = tile.top;

  if (auto v = tile_by_top.find(top); v != tile_by_top.end()) {
    (*v).second.push_back(tile_num);
  } else {
    tile_by_top.emplace(top, std::vector{tile_num});
  }

  const auto both = (left << 16) | top;

  if (auto v = tile_by_lefttop.find(both); v != tile_by_lefttop.end()) {
    (*v).second.push_back(tile_num);
  } else {
    tile_by_lefttop.emplace(both, std::vector{tile_num});
  }
}

void add_all(tile_t &&tile) {
  for (auto rotate_c = 0; rotate_c < 2; ++rotate_c) {
    add(tile);

    tile.flip_left_right();
    add(tile);

    tile.flip_top_bottom();
    add(tile);

    // l -> r -> l keeping t @ b
    tile.flip_left_right();
    add(tile);

    // return to normal
    tile.flip_top_bottom();

    tile.rotate_cw();
  }
}

using tiling_t = std::vector<tilenum_t>;

std::unordered_set<std::string> tilings{};

std::optional<tiling_t> put(const tilenum_t &tile_num, const tiling_t &tiling,
                            const std::unordered_set<tileid_t> &used) {
  const tilenum_t new_pos = tiling.size();
  const tile_t &tile = tiles[tile_num];

  tiling_t new_tiling = tiling;
  std::unordered_set<tileid_t> new_used = used;
  new_tiling.push_back(tile_num);
  if (new_tiling.size() == tiling_size)
    return new_tiling;
  new_used.insert(tile.id);

  const auto new_new_pos = new_pos + 1;

  std::vector<tilenum_t> *candidates = nullptr;

  if (new_new_pos % tiling_side && new_new_pos >= tiling_side) {
    auto key =
        (tile.right << 16) | tiles[tiling[new_new_pos - tiling_side]].bottom;

    auto f = tile_by_lefttop.find(key);
    if (f == tile_by_lefttop.end())
      return std::nullopt;
    else
      candidates = &(*f).second;
  } else if (new_new_pos % tiling_side) {
    auto f = tile_by_left.find(tile.right);
    if (f == tile_by_left.end())
      return std::nullopt;
    else
      candidates = &(*f).second;
  } else {
    auto bottom = tiles[tiling[new_new_pos - tiling_side]].bottom;
    auto f = tile_by_top.find(bottom);
    if (f == tile_by_top.end())
      return std::nullopt;
    else
      candidates = &(*f).second;
  }

  for (tilenum_t new_tile_num : *candidates) {
    if (new_used.count(tiles[new_tile_num].id))
      continue;
    if (auto rv = put(new_tile_num, new_tiling, new_used); rv)
      return rv;
  }

  return std::nullopt;
}

int main() {
  for (std::string tline; std::getline(std::cin, tline);) {
    tile_t tile{0, 0, std::string()};
    tile.id = (tileid_t)std::stoi(tline.substr(5));
    for (std::string line; std::getline(std::cin, line) && !line.empty();) {
      tile.side = (int)line.length();
      tile.data.reserve(tile.side * tile.side);

      std::copy(line.cbegin(), line.cend(), std::back_inserter(tile.data));
    }

    tile.compute_edges();
    add_all(std::move(tile));
  }

  // magic 8 is 4 flips times 0..1 rotates
  tiling_size = tiles.size() / 8;
  tiling_side = (int)std::sqrt(tiling_size);

  for (tilenum_t tilenum = 0; tilenum < tiles.size(); ++tilenum) {
    if (auto r = put(tilenum, tiling_t{}, std::unordered_set<tileid_t>{})) {
      for (size_t row = 0; row < tiling_side; ++row) {
        for (size_t col = 0; col < tiling_side; ++col) {
          const auto tn = r.value()[row * tiling_side + col];
          std::cout << tiles[tn].id << "(" << tn << ") ";
        }
        std::cout << std::endl;
      }

      uint64_t mul = (uint64_t)tiles[r.value()[0]].id *
                     tiles[r.value()[tiling_side - 1]].id *
                     tiles[r.value()[(tiling_side - 1) * tiling_side]].id *
                     tiles[r.value().back()].id;

      std::cout << mul << std::endl;

      break;
    }
  }

  return 0;
}
