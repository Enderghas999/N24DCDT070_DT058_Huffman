// ============================================================================
//  DT058 - Tao ma Huffman tu bang tan suat nho (INT13185)
//  Loi thuat toan dung chung cho server web va bo kiem tra.
//
//  Thuat toan duy nhat: THAM LAM, tim hai nut nho nhat bang cach QUET TUYEN
//  TINH danh sach ds (khong dung heap), giong src/huffman.cpp.
//
//  Moi ham nhan them con tro GhiVet* (co the la nullptr). Khi khac nullptr,
//  ham ghi lai tung su kien de trang web mo phong; khi bang nullptr, ham chay
//  "thuan" de do thoi gian. Hai che do dung CUNG MOT doan ma, nen dieu duoc
//  minh hoa chinh la dieu duoc do.
// ============================================================================
#pragma once

#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

namespace hf {

struct Nut {
  int trong_so; // tong tan suat cua cay con
  int la;       // chi so ky tu (0..m-1) neu la la, -1 neu la nut trong
  int trai, phai;
};

// ---------------------------------------------------------------------------
//  JSON toi gian (chi dung de ghi, khong can thu vien ngoai)
// ---------------------------------------------------------------------------
inline std::string json_str(const std::string &s) {
  std::string o = "\"";
  for (unsigned char c : s) {
    switch (c) {
    case '"': o += "\\\""; break;
    case '\\': o += "\\\\"; break;
    case '\n': o += "\\n"; break;
    case '\r': o += "\\r"; break;
    case '\t': o += "\\t"; break;
    default:
      if (c < 0x20) {
        char b[8];
        snprintf(b, sizeof b, "\\u%04x", c);
        o += b;
      } else
        o += (char)c;
    }
  }
  return o + "\"";
}

inline std::string json_ints(const std::vector<int> &v) {
  std::string o = "[";
  for (size_t i = 0; i < v.size(); i++)
    o += (i ? "," : "") + std::to_string(v[i]);
  return o + "]";
}

// ---------------------------------------------------------------------------
//  Ghi vet: moi su kien la mot anh chup day du trang thai, nen trang web lui
//  ve buoc truoc chi can hien lai su kien do (cay, ds, ngan xep, ma, nhat ky
//  luon cung mot trang thai).
// ---------------------------------------------------------------------------
struct Khung {        // mot khung trong ngan xep loi goi
  std::string ham;    // ten ham
  std::string tham_so; // mo ta tham so
};

struct GhiVet {
  // Trang thai hien tai do thuat toan cap nhat truoc khi goi ghi()
  const std::vector<Nut> *cay = nullptr;
  std::vector<int> ds;
  std::vector<Khung> ngan_xep;
  std::vector<std::string> ma;
  std::vector<std::string> nhan; // nhan ky tu (de viet loi giai thich)
  long long so_sanh = 0;
  int so_lan_ghep = 0;
  int so_loi_goi = 0;

  size_t gioi_han = 5000; // so su kien toi da (kiem soat khoi luong)
  bool bi_cat = false;
  std::vector<std::string> su_kien; // moi phan tu la mot doi tuong JSON

  // Nhan de doc cua mot nut: la -> "A(5)", nut trong -> "#6(14)"
  std::string ten(int id) const {
    const Nut &n = (*cay)[id];
    if (n.la >= 0)
      return nhan[n.la] + "(" + std::to_string(n.trong_so) + ")";
    return "#" + std::to_string(id) + "(" + std::to_string(n.trong_so) + ")";
  }

  // pha: khoi_tao | quet | chon | ghep | ma | tong
  // nhan_nut: cac cap "vai tro":id de to mau (dang_xet, nho1, nho2, moi, la_ma)
  void ghi(const std::string &pha, int dong, const std::string &loi,
           const std::vector<std::pair<std::string, int>> &to_nut = {},
           const std::vector<std::pair<std::string, int>> &to_o = {}) {
    if (su_kien.size() >= gioi_han) {
      bi_cat = true;
      return;
    }
    std::ostringstream o;
    o << "{\"pha\":" << json_str(pha) << ",\"dong\":" << dong
      << ",\"loi\":" << json_str(loi) << ",\"so_nut\":" << cay->size()
      << ",\"ds\":" << json_ints(ds) << ",\"so_sanh\":" << so_sanh
      << ",\"so_lan_ghep\":" << so_lan_ghep << ",\"so_loi_goi\":" << so_loi_goi
      << ",\"to_nut\":{";
    for (size_t i = 0; i < to_nut.size(); i++)
      o << (i ? "," : "") << json_str(to_nut[i].first) << ":" << to_nut[i].second;
    o << "},\"to_o\":{";
    for (size_t i = 0; i < to_o.size(); i++)
      o << (i ? "," : "") << json_str(to_o[i].first) << ":" << to_o[i].second;
    o << "},\"ngan_xep\":[";
    for (size_t i = 0; i < ngan_xep.size(); i++)
      o << (i ? "," : "") << "{\"ham\":" << json_str(ngan_xep[i].ham)
        << ",\"ts\":" << json_str(ngan_xep[i].tham_so) << "}";
    o << "],\"ma\":[";
    for (size_t i = 0; i < ma.size(); i++)
      o << (i ? "," : "") << json_str(ma[i]);
    o << "]}";
    su_kien.push_back(o.str());
  }
};

// ---------------------------------------------------------------------------
//  1. Quet danh sach ds, tim vi tri nut nho nhat (vt1) va nho nhi (vt2).
//     Khi hoa, giu nut dung TRUOC trong ds (chi cap nhat khi nho hon han).
//     So phep so sanh voi k = |ds| la (k-1) + (k-2) = 2k - 3.
// ---------------------------------------------------------------------------
inline void lay_hai_nho_nhat(const std::vector<Nut> &cay,
                             const std::vector<int> &ds, int &vt1, int &vt2,
                             long long &dem, GhiVet *gv = nullptr) {
  const int k = (int)ds.size();
  vt1 = 0;
  if (gv) {
    gv->ngan_xep.push_back({"lay_hai_nho_nhat", "luot 1"});
    gv->ghi("quet", 4,
            "Lượt 1 tìm nút nhỏ nhất: tạm coi vt1 = 0, tức " + gv->ten(ds[0]) +
                ".",
            {{"nho1", ds[0]}}, {{"vt1", 0}});
  }
  for (int i = 1; i < k; i++) {
    dem++;
    const bool nho_hon = cay[ds[i]].trong_so < cay[ds[vt1]].trong_so;
    if (gv) {
      gv->so_sanh = dem;
      gv->ngan_xep.back().tham_so = "luot 1, i = " + std::to_string(i);
      std::string loi = "So sánh ds[" + std::to_string(i) + "] = " +
                        gv->ten(ds[i]) + " với nhỏ nhất hiện tại " +
                        gv->ten(ds[vt1]) + ": ";
      if (nho_hon)
        loi += std::to_string(cay[ds[i]].trong_so) + " < " +
               std::to_string(cay[ds[vt1]].trong_so) + " nên cập nhật vt1 = " +
               std::to_string(i) + ".";
      else if (cay[ds[i]].trong_so == cay[ds[vt1]].trong_so)
        loi += "bằng nhau, giữ nút đứng trước (vt1 = " + std::to_string(vt1) +
               ").";
      else
        loi += std::to_string(cay[ds[i]].trong_so) + " > " +
               std::to_string(cay[ds[vt1]].trong_so) + " nên giữ vt1 = " +
               std::to_string(vt1) + ".";
      gv->ghi("quet", 5, loi,
              {{"dang_xet", ds[i]}, {"nho1", nho_hon ? ds[i] : ds[vt1]}},
              {{"i", i}, {"vt1", nho_hon ? i : vt1}});
    }
    if (nho_hon)
      vt1 = i;
  }
  vt2 = (vt1 == 0) ? 1 : 0;
  if (gv) {
    gv->ngan_xep.back().tham_so = "luot 2";
    gv->ghi("quet", 6,
            "Lượt 2 tìm nút nhỏ nhì: tạm coi vt2 = " + std::to_string(vt2) +
                ", tức " + gv->ten(ds[vt2]) + " (vị trí đầu tiên khác vt1).",
            {{"nho1", ds[vt1]}, {"nho2", ds[vt2]}}, {{"vt1", vt1}, {"vt2", vt2}});
  }
  for (int i = 0; i < k; i++) {
    if (i == vt1 || i == vt2) {
      if (gv) {
        gv->ngan_xep.back().tham_so = "luot 2, i = " + std::to_string(i);
        gv->ghi("quet", 7,
                "Bỏ qua ds[" + std::to_string(i) + "] = " + gv->ten(ds[i]) +
                    (i == vt1
                         ? " vì đó là vt1, nút nhỏ nhất đã chọn ở lượt 1."
                         : " vì đó chính là vị trí khởi tạo vt2, không tự so "
                           "với chính nó."),
                {{"nho1", ds[vt1]}, {"nho2", ds[vt2]}},
                {{"i", i}, {"vt1", vt1}, {"vt2", vt2}});
      }
      continue;
    }
    dem++;
    const bool nho_hon = cay[ds[i]].trong_so < cay[ds[vt2]].trong_so;
    if (gv) {
      gv->so_sanh = dem;
      gv->ngan_xep.back().tham_so = "luot 2, i = " + std::to_string(i);
      std::string loi = "So sánh ds[" + std::to_string(i) + "] = " +
                        gv->ten(ds[i]) + " với nhỏ nhì hiện tại " +
                        gv->ten(ds[vt2]) + ": ";
      if (nho_hon)
        loi += "nhỏ hơn nên cập nhật vt2 = " + std::to_string(i) + ".";
      else if (cay[ds[i]].trong_so == cay[ds[vt2]].trong_so)
        loi += "bằng nhau, giữ nút đứng trước (vt2 = " + std::to_string(vt2) +
               ").";
      else
        loi += "lớn hơn nên giữ vt2 = " + std::to_string(vt2) + ".";
      gv->ghi("quet", 7, loi,
              {{"dang_xet", ds[i]},
               {"nho1", ds[vt1]},
               {"nho2", nho_hon ? ds[i] : ds[vt2]}},
              {{"i", i}, {"vt1", vt1}, {"vt2", nho_hon ? i : vt2}});
    }
    if (nho_hon)
      vt2 = i;
  }
  if (gv)
    gv->ngan_xep.pop_back();
}

// ---------------------------------------------------------------------------
//  2. Dung cay Huffman. La i (0..m-1) ung voi ky tu thu i. Tra ve chi so goc.
//     tong_noi: tong trong so cac nut cha (= tong so bit cua ma toi uu).
// ---------------------------------------------------------------------------
inline int tao_cay_huffman(const std::vector<int> &f, std::vector<Nut> &cay,
                           long long &dem, long long &tong_noi,
                           GhiVet *gv = nullptr) {
  const int m = (int)f.size();
  cay.clear();
  cay.reserve(2 * m);
  std::vector<int> ds;
  ds.reserve(m);
  for (int i = 0; i < m; i++) {
    cay.push_back({f[i], i, -1, -1});
    ds.push_back(i);
  }
  tong_noi = 0;
  if (gv) {
    gv->cay = &cay;
    gv->ds = ds;
    gv->ngan_xep = {{"tao_cay_huffman", "m = " + std::to_string(m)}};
    std::string loi = "Tạo " + std::to_string(m) +
                      " nút lá, mỗi lá mang trọng số bằng tần suất; ds = [";
    for (int i = 0; i < m; i++)
      loi += (i ? ", " : "") + gv->ten(i);
    loi += "].";
    if (m == 1)
      loi += " ds chỉ có một nút nên không cần ghép; lá này cũng là gốc.";
    gv->ghi("khoi_tao", 2, loi);
  }
  while (ds.size() > 1) {
    if (gv)
      gv->ghi("quet", 3,
              "ds còn " + std::to_string(ds.size()) +
                  " nút (> 1) nên tiếp tục: tìm hai nút có trọng số nhỏ nhất.");
    int a, b;
    lay_hai_nho_nhat(cay, ds, a, b, dem, gv);
    const int x = ds[a], y = ds[b];
    if (gv)
      gv->ghi("chon", 8,
              "Hai nút nhẹ nhất là " + gv->ten(x) + " (vt1 = " +
                  std::to_string(a) + ") và " + gv->ten(y) + " (vt2 = " +
                  std::to_string(b) + "). Ghép chúng thành một nút cha.",
              {{"nho1", x}, {"nho2", y}}, {{"vt1", a}, {"vt2", b}});
    cay.push_back({cay[x].trong_so + cay[y].trong_so, -1, x, y});
    const int cha = (int)cay.size() - 1;
    tong_noi += cay[cha].trong_so;
    const int lon = a > b ? a : b, nho = a > b ? b : a;
    ds.erase(ds.begin() + lon);
    ds.erase(ds.begin() + nho);
    ds.push_back(cha);
    if (gv) {
      gv->so_lan_ghep++;
      gv->ds = ds;
      gv->ghi("ghep", 9,
              "Tạo nút cha #" + std::to_string(cha) + " có trọng số " +
                  std::to_string(cay[x].trong_so) + " + " +
                  std::to_string(cay[y].trong_so) + " = " +
                  std::to_string(cay[cha].trong_so) + "; " + gv->ten(x) +
                  " là con trái (bit 0), " + gv->ten(y) +
                  " là con phải (bit 1). Xóa hai nút khỏi ds, thêm #" +
                  std::to_string(cha) + " vào cuối. ds còn " +
                  std::to_string(ds.size()) + " nút.",
              {{"moi", cha}, {"nho1", x}, {"nho2", y}},
              {{"moi", (int)ds.size() - 1}});
    }
  }
  if (gv) {
    gv->ghi("ghep", 10,
            "ds chỉ còn một nút " + gv->ten(ds[0]) +
                ", đó là gốc. Kết thúc dựng cây sau " +
                std::to_string(gv->so_lan_ghep) + " lần ghép.",
            {{"moi", ds[0]}});
    gv->ngan_xep.clear();
  }
  return ds[0];
}

// ---------------------------------------------------------------------------
//  3. Duyet cay tu goc: sang trai them '0', sang phai them '1'.
//     Mot ky tu duy nhat (goc la la) nhan ma "0".
// ---------------------------------------------------------------------------
inline void tao_ma(const std::vector<Nut> &cay, int nut,
                   const std::string &tien_to, std::vector<std::string> &ma,
                   int &so_goi, GhiVet *gv = nullptr) {
  so_goi++;
  const Nut &n = cay[nut];
  if (gv) {
    gv->so_loi_goi = so_goi;
    gv->ngan_xep.push_back(
        {"tao_ma", gv->ten(nut) + ", \"" + tien_to + "\""});
  }
  if (n.trai == -1) {
    ma[n.la] = tien_to.empty() ? "0" : tien_to;
    if (gv) {
      gv->ma[n.la] = ma[n.la];
      gv->ghi("ma", 12,
              gv->ten(nut) + " là lá nên gán mã \"" + ma[n.la] + "\"" +
                  (tien_to.empty()
                       ? " (cây chỉ có một lá, tiền tố rỗng nên dùng \"0\")."
                       : ", chính là đường đi từ gốc xuống lá này.") +
                  " Trả về hàm gọi.",
              {{"la_ma", nut}});
      gv->ngan_xep.pop_back();
    }
    return;
  }
  if (gv)
    gv->ghi("ma", 13,
            gv->ten(nut) + " là nút trong: đi sang con trái " +
                gv->ten(n.trai) + " với tiền tố \"" + tien_to + "0\".",
            {{"dang_xet", nut}, {"nho1", n.trai}});
  tao_ma(cay, n.trai, tien_to + "0", ma, so_goi, gv);
  if (gv)
    gv->ghi("ma", 14,
            "Quay lại " + gv->ten(nut) +
                ", con trái đã xong; đi sang con phải " + gv->ten(n.phai) +
                " với tiền tố \"" + tien_to + "1\".",
            {{"dang_xet", nut}, {"nho2", n.phai}});
  tao_ma(cay, n.phai, tien_to + "1", ma, so_goi, gv);
  if (gv)
    gv->ngan_xep.pop_back();
}

// 4. Tong so bit = sum f[i] * |ma[i]|
inline long long tong_so_bit(const std::vector<std::string> &ma,
                             const std::vector<int> &f) {
  long long t = 0;
  for (size_t i = 0; i < ma.size(); i++)
    t += (long long)f[i] * (long long)ma[i].size();
  return t;
}

// 5. Khong ma nao la tien to cua ma khac
inline bool khong_tien_to(const std::vector<std::string> &ma) {
  for (size_t i = 0; i < ma.size(); i++)
    for (size_t j = 0; j < ma.size(); j++)
      if (i != j && ma[i].size() <= ma[j].size() &&
          ma[j].compare(0, ma[i].size(), ma[i]) == 0)
        return false;
  return true;
}

// 6. Ma hoa / giai ma chuoi thu. chi_so[c] = vi tri cua ky tu c, -1 neu khong co
inline std::string ma_hoa(const std::string &s, const std::vector<char> &kt,
                          const std::vector<std::string> &ma) {
  std::string kq;
  for (char c : s)
    for (size_t i = 0; i < kt.size(); i++)
      if (kt[i] == c) {
        kq += ma[i];
        break;
      }
  return kq;
}

inline std::string giai_ma(const std::string &bits, const std::vector<Nut> &cay,
                           int goc, const std::vector<char> &kt) {
  std::string kq;
  if (cay[goc].trai == -1) { // mot ky tu: moi bit '0' la mot ky tu
    for (size_t i = 0; i < bits.size(); i++)
      kq += kt[cay[goc].la];
    return kq;
  }
  int cur = goc;
  for (char b : bits) {
    cur = (b == '0') ? cay[cur].trai : cay[cur].phai;
    if (cay[cur].trai == -1) {
      kq += kt[cay[cur].la];
      cur = goc;
    }
  }
  return kq;
}

// ---------------------------------------------------------------------------
//  Kiem tra dau vao theo dieu kien phieu: m trong 1..8, ky tu ASCII in duoc
//  (33..126) khong trung, tan suat nguyen trong 1..100, chuoi thu chi gom ky
//  tu co trong bang. Tra ve chuoi rong neu hop le, nguoc lai la thong bao loi.
// ---------------------------------------------------------------------------
const int M_TOI_DA = 8, F_TOI_THIEU = 1, F_TOI_DA = 100, CHUOI_TOI_DA = 64;

inline std::string doc_dau_vao(const std::string &kt_raw, const std::string &f_raw,
                               const std::string &s, std::vector<char> &kt,
                               std::vector<int> &f) {
  kt.clear();
  f.clear();
  {
    std::istringstream is(kt_raw);
    std::string tok;
    while (is >> tok) {
      if (tok.size() != 1)
        return "Mỗi ký tự phải là đúng một ký tự, các ký tự cách nhau bằng dấu "
               "cách (\"" + tok + "\" không hợp lệ).";
      unsigned char c = (unsigned char)tok[0];
      if (c < 33 || c > 126)
        return "Ký tự phải là ký tự ASCII in được (không dấu, không khoảng "
               "trắng).";
      for (char d : kt)
        if (d == (char)c)
          return std::string("Ký tự '") + (char)c +
                 "' bị trùng; các ký tự phải khác nhau.";
      kt.push_back((char)c);
    }
  }
  if (kt.empty())
    return "Cần nhập ít nhất 1 ký tự (m = 0 không hợp lệ).";
  if ((int)kt.size() > M_TOI_DA)
    return "Số ký tự m = " + std::to_string(kt.size()) +
           " vượt giới hạn của phiếu (1 ≤ m ≤ 8).";
  {
    std::istringstream is(f_raw);
    std::string tok;
    while (is >> tok) {
      bool so = !tok.empty() && tok.size() <= 4;
      for (char c : tok)
        if (c < '0' || c > '9')
          so = false;
      int v = so ? std::stoi(tok) : -1;
      if (!so || v < F_TOI_THIEU || v > F_TOI_DA)
        return "Tần suất \"" + tok +
               "\" không hợp lệ: phải là số nguyên từ 1 đến 100.";
      f.push_back(v);
    }
  }
  if (f.size() != kt.size())
    return "Cần đúng " + std::to_string(kt.size()) + " tần suất (đã nhập " +
           std::to_string(f.size()) + ").";
  if ((int)s.size() > CHUOI_TOI_DA)
    return "Chuỗi thử dài tối đa 64 ký tự.";
  for (char c : s) {
    bool co = false;
    for (char d : kt)
      if (d == c)
        co = true;
    if (!co)
      return std::string("Chuỗi thử có ký tự '") + c +
             "' không nằm trong bảng; hãy sửa hoặc để trống.";
  }
  return "";
}

// ---------------------------------------------------------------------------
//  Chay tron quy trinh (khong ghi vet) - dung cho do thoi gian va kiem tra.
// ---------------------------------------------------------------------------
struct KetQua {
  std::vector<Nut> cay;
  int goc = 0;
  std::vector<std::string> ma;
  long long tong_bit = 0, tong_noi = 0, so_sanh = 0;
  int so_loi_goi = 0;
};

inline KetQua chay(const std::vector<int> &f, GhiVet *gv = nullptr) {
  KetQua r;
  r.goc = tao_cay_huffman(f, r.cay, r.so_sanh, r.tong_noi, gv);
  r.ma.assign(f.size(), "");
  tao_ma(r.cay, r.goc, "", r.ma, r.so_loi_goi, gv);
  r.tong_bit = tong_so_bit(r.ma, f);
  return r;
}

} // namespace hf
