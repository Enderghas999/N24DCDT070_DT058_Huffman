// ============================================================================
//  Bo kiem tra tinh dung cho DT058 (dung chung cho tests/kiem_tra.cpp va
//  API /api/kiem_tra cua server).
//
//  Nguon doi chieu doc lap (oracle): chi_phi_toi_uu() duyet moi bo do dai ma
//  thoa bat dang thuc Kraft va tra ve tong bit nho nhat co the. Ham nay KHONG
//  dung quy tac ghep tham lam, chi dung de kiem tra; trang web khong dung no
//  lam thuat toan.
// ============================================================================
#pragma once

#include "huffman_core.hpp"

#include <algorithm>
#include <random>
#include <string>
#include <vector>

namespace hf {

struct PhepKiem {
  std::string mo_ta;
  bool dat;
  std::string chi_tiet;
};

struct CaKiem {
  std::string ma_ca, nhom, mo_ta;
  std::vector<PhepKiem> kiem;
  bool dat() const {
    for (auto &k : kiem)
      if (!k.dat)
        return false;
    return true;
  }
};

// Oracle: tong bit nho nhat cua MOI ma tien to co the co.
// Theo bat dang thuc Kraft, mot bo do dai l[i] co ma tien to tuong ung khi va
// chi khi sum 2^-l[i] <= 1. Ma toi uu gan do dai ngan hon cho tan suat lon hon,
// nen chi can duyet moi day do dai KHONG GIAM (1 <= l <= m-1) ghep voi tan
// suat da sap GIAM DAN, roi lay tong f[i]*l[i] nho nhat. Cach nay khong dung
// quy tac ghep tham lam nen la nguon doi chieu doc lap.
inline void duyet_do_dai(const std::vector<int> &f_giam, size_t i, int l_min,
                         int L, unsigned long long da_dung,
                         long long chi_phi, long long &best) {
  if (best >= 0 && chi_phi >= best)
    return;
  if (i == f_giam.size()) {
    best = chi_phi;
    return;
  }
  for (int l = l_min; l <= L; l++) {
    unsigned long long w = 1ULL << (L - l);
    // cac phan tu con lai dai toi da L nen moi phan tu ton it nhat 2^0 = 1
    if (da_dung + w + (f_giam.size() - i - 1) > (1ULL << L))
      continue;
    duyet_do_dai(f_giam, i + 1, l, L, da_dung + w,
                 chi_phi + (long long)f_giam[i] * l, best);
  }
}

inline long long chi_phi_toi_uu(std::vector<int> f) {
  if (f.size() <= 1)
    return f.empty() ? 0 : f[0]; // quy uoc mot ky tu: ma "0" dai 1 bit
  std::sort(f.begin(), f.end(), [](int a, int b) { return a > b; });
  long long best = -1;
  duyet_do_dai(f, 0, 1, (int)f.size() - 1, 0, 0, best);
  return best;
}

// Tong Kraft: sum 2^-l[i] == 1  <=>  sum 2^(L - l[i]) == 2^L
inline bool kraft_bang_1(const std::vector<std::string> &ma) {
  size_t L = 0;
  for (auto &s : ma)
    L = std::max(L, s.size());
  if (ma.size() == 1)
    return ma[0] == "0"; // quy uoc mot ky tu
  unsigned long long tong = 0;
  for (auto &s : ma)
    tong += 1ULL << (L - s.size());
  return tong == (1ULL << L);
}

inline std::string noi_ma(const std::vector<std::string> &ma) {
  std::string s;
  for (size_t i = 0; i < ma.size(); i++)
    s += (i ? ", " : "") + ma[i];
  return s;
}

// Chay mot bang tan suat co dinh va tra ve cac phep kiem chung
inline CaKiem ca_bang(const std::string &ma_ca, const std::string &nhom,
                      const std::string &mo_ta, const std::string &kt_raw,
                      const std::string &f_raw, const std::string &s,
                      long long bit_mong_doi,
                      const std::vector<std::string> &ma_mong_doi = {},
                      const std::string &chuoi_bit_mong_doi = "") {
  CaKiem c{ma_ca, nhom, mo_ta, {}};
  std::vector<char> kt;
  std::vector<int> f;
  std::string loi = doc_dau_vao(kt_raw, f_raw, s, kt, f);
  c.kiem.push_back({"đầu vào hợp lệ", loi.empty(), loi});
  if (!loi.empty())
    return c;
  KetQua r = chay(f);
  const int m = (int)f.size();
  long long tu = chi_phi_toi_uu(f);
  c.kiem.push_back({"tổng bit = " + std::to_string(bit_mong_doi),
                    r.tong_bit == bit_mong_doi,
                    "thực tế " + std::to_string(r.tong_bit)});
  c.kiem.push_back({"bằng tổng bit tối ưu của oracle (duyệt độ dài mã)",
                    r.tong_bit == tu, "oracle " + std::to_string(tu)});
  c.kiem.push_back({"không mã nào là tiền tố của mã khác",
                    khong_tien_to(r.ma), noi_ma(r.ma)});
  c.kiem.push_back({"số phép so sánh = (m − 1)² = " +
                        std::to_string((long long)(m - 1) * (m - 1)),
                    r.so_sanh == (long long)(m - 1) * (m - 1),
                    "thực tế " + std::to_string(r.so_sanh)});
  if (!ma_mong_doi.empty())
    c.kiem.push_back({"bảng mã = " + noi_ma(ma_mong_doi), r.ma == ma_mong_doi,
                      "thực tế " + noi_ma(r.ma)});
  if (!s.empty()) {
    std::string bits = ma_hoa(s, kt, r.ma);
    if (!chuoi_bit_mong_doi.empty())
      c.kiem.push_back({"chuỗi bit = " + chuoi_bit_mong_doi,
                        bits == chuoi_bit_mong_doi, "thực tế " + bits});
    std::string lai = giai_ma(bits, r.cay, r.goc, kt);
    c.kiem.push_back({"giải mã lại được \"" + s + "\"", lai == s,
                      "thực tế \"" + lai + "\""});
  }
  return c;
}

inline CaKiem ca_loi(const std::string &ma_ca, const std::string &mo_ta,
                     const std::string &kt_raw, const std::string &f_raw,
                     const std::string &s) {
  CaKiem c{ma_ca, "Đầu vào không hợp lệ", mo_ta, {}};
  std::vector<char> kt;
  std::vector<int> f;
  std::string loi = doc_dau_vao(kt_raw, f_raw, s, kt, f);
  c.kiem.push_back({"bị từ chối kèm thông báo", !loi.empty(),
                    loi.empty() ? "không bị từ chối" : loi});
  return c;
}

inline std::vector<CaKiem> chay_bo_kiem_tra(int so_bang_ngau_nhien = 2000,
                                            unsigned hat_giong = 2026) {
  std::vector<CaKiem> cac;

  // --- Ví dụ của đề và ca tự thiết kế ---
  cac.push_back(ca_bang("VD01", "Ví dụ của đề", "A B C D, f = 5 9 12 13",
                        "A B C D", "5 9 12 13", "ABCD", 78,
                        {"00", "01", "10", "11"}, "00011011"));
  cac.push_back(ca_bang("VD02", "Ví dụ của đề", "A B, f = 2 3, chuỗi ABBA",
                        "A B", "2 3", "ABBA", 5, {"0", "1"}, "0110"));
  cac.push_back(ca_bang("TK01", "Tự thiết kế",
                        "Tần suất không sắp tăng: f = 1 4 2 5", "A B C D",
                        "1 4 2 5", "ACDB", 22, {"100", "11", "101", "0"},
                        "100101011"));
  cac.push_back(ca_bang("TK02", "Tự thiết kế", "Nhiều trọng số hòa: 2 2 4 4 8",
                        "A B C D E", "2 2 4 4 8", "EDCBA", 44));
  cac.push_back(ca_bang("TK03", "Tự thiết kế",
                        "Dãy Fibonacci 1 1 2 3 5 8 13 21: cây lệch, mã dài m − 1",
                        "A B C D E F G H", "1 1 2 3 5 8 13 21", "AH", 132,
                        {"1111110", "1111111", "111110", "11110", "1110",
                         "110", "10", "0"}));
  cac.push_back(ca_bang("KT09", "Tự thiết kế", "Tần suất lệch mạnh: 1 1 8",
                        "A B C", "1 1 8", "CCAB", 12, {"00", "01", "1"}));

  // --- Trường hợp biên ---
  cac.push_back(ca_bang("BIEN01", "Biên", "m = 1: mã của ký tự duy nhất là 0",
                        "A", "7", "AAA", 7, {"0"}, "000"));
  cac.push_back(ca_bang("BIEN02", "Biên", "m = 2 (nhỏ nhất có ghép)", "X Y",
                        "100 1", "XYX", 101, {"1", "0"}, "101"));
  cac.push_back(ca_bang("BIEN03", "Biên",
                        "m = 8, mọi f = 5: cây cân bằng, mọi mã dài 3",
                        "A B C D E F G H", "5 5 5 5 5 5 5 5", "", 120,
                        {"000", "001", "010", "011", "100", "101", "110",
                         "111"}));
  cac.push_back(ca_bang("BIEN04", "Biên", "f nhỏ nhất, hòa toàn bộ: 1 1 1",
                        "A B C", "1 1 1", "ABC", 5, {"10", "11", "0"}));
  cac.push_back(ca_bang("BIEN05", "Biên", "m = 8, mọi f = 100 (lớn nhất)",
                        "A B C D E F G H",
                        "100 100 100 100 100 100 100 100", "", 2400));

  // --- Hàm con ---
  {
    CaKiem c{"HAM01", "Hàm con",
             "lay_hai_nho_nhat khi hòa, trọng số 4 2 2 5", {}};
    std::vector<Nut> cay = {{4, 0, -1, -1}, {2, 1, -1, -1}, {2, 2, -1, -1},
                            {5, 3, -1, -1}};
    std::vector<int> ds = {0, 1, 2, 3};
    int a = -1, b = -1;
    long long dem = 0;
    lay_hai_nho_nhat(cay, ds, a, b, dem);
    c.kiem.push_back({"vt1 = 1 (nút đứng trước khi hòa)", a == 1,
                      "thực tế " + std::to_string(a)});
    c.kiem.push_back({"vt2 = 2", b == 2, "thực tế " + std::to_string(b)});
    c.kiem.push_back({"đúng 2k − 3 = 5 phép so sánh", dem == 5,
                      "thực tế " + std::to_string(dem)});
    cac.push_back(c);
  }
  {
    CaKiem c{"HAM02", "Hàm con", "khong_tien_to với bộ mã sai và bộ mã đúng",
             {}};
    c.kiem.push_back({"{0, 01, 11} trả về sai", !khong_tien_to({"0", "01", "11"}),
                      "0 là tiền tố của 01"});
    c.kiem.push_back({"{0, 10, 11} trả về đúng",
                      khong_tien_to({"0", "10", "11"}), ""});
    cac.push_back(c);
  }
  {
    // Vết mô phỏng phải khớp kết quả chạy thuần (cùng một đoạn mã)
    CaKiem c{"VET01", "Mô phỏng",
             "Vết của trang web khớp kết quả chạy thuần (f = 5 9 12 13 16)", {}};
    std::vector<int> f = {5, 9, 12, 13, 16};
    GhiVet gv;
    gv.nhan = {"A", "B", "C", "D", "E"};
    gv.ma.assign(f.size(), "");
    KetQua a = chay(f, &gv), b = chay(f);
    c.kiem.push_back({"bảng mã giống nhau", a.ma == b.ma, noi_ma(a.ma)});
    c.kiem.push_back({"bộ đếm so sánh giống nhau", gv.so_sanh == b.so_sanh,
                      std::to_string(gv.so_sanh) + " / " +
                          std::to_string(b.so_sanh)});
    c.kiem.push_back({"tổng bit = 124", b.tong_bit == 124,
                      "thực tế " + std::to_string(b.tong_bit)});
    c.kiem.push_back({"số lời gọi tao_ma = 2m − 1 = 9", b.so_loi_goi == 9,
                      "thực tế " + std::to_string(b.so_loi_goi)});
    cac.push_back(c);
  }

  // --- Đầu vào không hợp lệ ---
  cac.push_back(ca_loi("LOI01", "m = 0 (không có ký tự)", "", "", ""));
  cac.push_back(ca_loi("LOI02", "m = 9 vượt giới hạn",
                       "A B C D E F G H I", "1 1 1 1 1 1 1 1 1", ""));
  cac.push_back(ca_loi("LOI03", "Ký tự trùng", "A A", "3 4", ""));
  cac.push_back(ca_loi("LOI04", "Tần suất 0", "A B", "0 4", ""));
  cac.push_back(ca_loi("LOI05", "Tần suất 101", "A B", "101 4", ""));
  cac.push_back(ca_loi("LOI06", "Tần suất không phải số", "A B", "3 x", ""));
  cac.push_back(ca_loi("LOI07", "Số tần suất khác số ký tự", "A B C", "3 4",
                       ""));
  cac.push_back(ca_loi("LOI08", "Ký tự nhiều chữ (AB)", "AB C", "3 4", ""));
  cac.push_back(ca_loi("LOI09", "Ký tự tiếng Việt có dấu (nhiều byte)",
                       "\xC4\x82 B", "3 4", ""));
  cac.push_back(ca_loi("LOI10", "Chuỗi thử có ký tự ngoài bảng", "A B", "3 4",
                       "ABZ"));

  // --- Tính chất trên bảng ngẫu nhiên ---
  {
    std::mt19937 rng(hat_giong);
    const char *ten[] = {"m mã, mỗi mã khác rỗng",
                         "không mã nào là tiền tố của mã khác",
                         "Σ 2^−độ dài = 1 (cây đầy đủ)",
                         "tổng bit = tổng trọng số các nút cha",
                         "tổng bit = tối ưu theo oracle duyệt độ dài mã",
                         "giải mã(mã hóa(s)) = s",
                         "số phép so sánh = (m − 1)²",
                         "trọng số gốc = Σ f"};
    int dat[8] = {0}, n = so_bang_ngau_nhien;
    std::string vi_du_sai[8];
    for (int t = 0; t < n; t++) {
      int m = 1 + (int)(rng() % 8);
      std::vector<int> f(m);
      std::vector<char> kt(m);
      for (int i = 0; i < m; i++) {
        f[i] = 1 + (int)(rng() % 100);
        kt[i] = (char)('A' + i);
      }
      std::string s;
      int len = (int)(rng() % 12);
      for (int i = 0; i < len; i++)
        s += kt[rng() % m];
      KetQua r = chay(f);
      bool ok[8];
      ok[0] = (int)r.ma.size() == m;
      for (auto &x : r.ma)
        if (x.empty())
          ok[0] = false;
      ok[1] = khong_tien_to(r.ma);
      ok[2] = kraft_bang_1(r.ma);
      ok[3] = r.tong_bit == r.tong_noi || m == 1;
      ok[4] = r.tong_bit == chi_phi_toi_uu(f);
      ok[5] = giai_ma(ma_hoa(s, kt, r.ma), r.cay, r.goc, kt) == s;
      ok[6] = r.so_sanh == (long long)(m - 1) * (m - 1);
      long long sf = 0;
      for (int v : f)
        sf += v;
      ok[7] = r.cay[r.goc].trong_so == sf;
      for (int k = 0; k < 8; k++) {
        if (ok[k])
          dat[k]++;
        else if (vi_du_sai[k].empty()) {
          vi_du_sai[k] = "f =";
          for (int v : f)
            vi_du_sai[k] += " " + std::to_string(v);
        }
      }
    }
    CaKiem c{"NN01", "Tính chất",
             std::to_string(n) + " bảng ngẫu nhiên (m = 1..8, f = 1..100, hạt "
             "giống " + std::to_string(hat_giong) + ")",
             {}};
    for (int k = 0; k < 8; k++)
      c.kiem.push_back({ten[k], dat[k] == n,
                        std::to_string(dat[k]) + "/" + std::to_string(n) +
                            (vi_du_sai[k].empty() ? "" : "; sai ở " + vi_du_sai[k])});
    cac.push_back(c);
  }
  return cac;
}

} // namespace hf
