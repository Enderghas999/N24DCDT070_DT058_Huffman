// Bo kiem tra tu dong DT058 - chay: tests/kiem_tra.exe [so_bang_ngau_nhien] [hat_giong]
// In tung ca, tung phep kiem; ma thoat 0 neu tat ca dat, 1 neu co ca sai.
#include "../bo_kiem_tra.hpp"
#include <chrono>
#include <cstdlib>
#include <iostream>

int main(int argc, char **argv) {
  int n = argc > 1 ? std::atoi(argv[1]) : 2000;
  unsigned seed = argc > 2 ? (unsigned)std::atoi(argv[2]) : 2026u;
  auto t0 = std::chrono::steady_clock::now();
  auto cac = hf::chay_bo_kiem_tra(n, seed);
  double ms = std::chrono::duration<double, std::milli>(
                  std::chrono::steady_clock::now() - t0).count();
  int ca_dat = 0, pk = 0, pk_dat = 0;
  for (auto &c : cac) {
    bool d = c.dat();
    ca_dat += d;
    std::cout << (d ? "[DAT] " : "[SAI] ") << c.ma_ca << " | " << c.nhom
              << " | " << c.mo_ta << "\n";
    for (auto &k : c.kiem) {
      pk++;
      pk_dat += k.dat;
      std::cout << "    " << (k.dat ? "dat " : "SAI ") << k.mo_ta
                << (k.chi_tiet.empty() ? "" : "  -> " + k.chi_tiet) << "\n";
    }
  }
  std::cout << "\nTONG KET: " << ca_dat << "/" << cac.size() << " ca dat, "
            << pk_dat << "/" << pk << " phep kiem dat (" << ms << " ms)\n";
  return ca_dat == (int)cac.size() ? 0 : 1;
}
