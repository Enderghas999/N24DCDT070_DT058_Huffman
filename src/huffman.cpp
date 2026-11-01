// DT058 - Tao ma Huffman tu bang tan suat nho (INT13185)
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Nut {
  int trong_so;   // tong tan suat cua cay con
  char kt;        // ky tu (chi co y nghia voi la)
  int trai, phai; // chi so nut con, -1 neu la la
};

long long dem_so_sanh = 0; // dem so phep so sanh trong luong

// 1. Quet danh sach ds (chua chi so nut), tim hai nut co trong so nho nhat.
//    Khi hoa, uu tien nut dung truoc trong danh sach. vt1: nho nhat, vt2: nho
//    nhi.
void lay_hai_nho_nhat(const vector<Nut> &cay, const vector<int> &ds, int &vt1,
                      int &vt2) {
  vt1 = 0;
  for (int i = 1; i < (int)ds.size(); i++) {
    dem_so_sanh++;
    if (cay[ds[i]].trong_so < cay[ds[vt1]].trong_so)
      vt1 = i;
  }
  vt2 = (vt1 == 0) ? 1 : 0;
  for (int i = 0; i < (int)ds.size(); i++) {
    if (i == vt1 || i == vt2)
      continue;
    dem_so_sanh++;
    if (cay[ds[i]].trong_so < cay[ds[vt2]].trong_so)
      vt2 = i;
  }
}

// 2. Dung cay Huffman. Nut la i (0..m-1) ung voi ky tu thu i. Tra ve chi so
// goc.
int tao_cay_huffman(const vector<char> &ky_tu, const vector<int> &f,
                    vector<Nut> &cay) {
  int m = ky_tu.size();
  cay.clear();
  vector<int> ds;
  for (int i = 0; i < m; i++) {
    cay.push_back({f[i], ky_tu[i], -1, -1});
    ds.push_back(i);
  }
  while (ds.size() > 1) {
    int a, b;
    lay_hai_nho_nhat(cay, ds, a, b);
    int x = ds[a], y = ds[b];
    cay.push_back(
        {cay[x].trong_so + cay[y].trong_so, 0, x, y}); // x trai, y phai
    int cha = cay.size() - 1;
    // thay hai nut bang nut cha: xoa vi tri lon truoc roi nho
    int lon = max(a, b), nho = min(a, b);
    ds.erase(ds.begin() + lon);
    ds.erase(ds.begin() + nho);
    ds.push_back(cha);
  }
  return ds[0];
}

// 3. Duyet cay: trai them '0', phai them '1'. ma[i] la ma cua la i.
void tao_ma(const vector<Nut> &cay, int nut, const string &tien_to,
            vector<string> &ma) {
  if (cay[nut].trai == -1) {                   // la
    ma[nut] = tien_to.empty() ? "0" : tien_to; // truong hop mot ky tu: ma "0"
    return;
  }
  tao_ma(cay, cay[nut].trai, tien_to + "0", ma);
  tao_ma(cay, cay[nut].phai, tien_to + "1", ma);
}

// 4. Tong so bit = sum(tan suat * do dai ma)
long long tong_so_bit(const vector<string> &ma, const vector<int> &f) {
  long long tong = 0;
  for (size_t i = 0; i < ma.size(); i++)
    tong += (long long)f[i] * ma[i].size();
  return tong;
}

// Kiem tra khong ma nao la tien to cua ma khac
bool khong_tien_to(const vector<string> &ma) {
  for (size_t i = 0; i < ma.size(); i++)
    for (size_t j = 0; j < ma.size(); j++)
      if (i != j && ma[i].size() <= ma[j].size() &&
          ma[j].compare(0, ma[i].size(), ma[i]) == 0)
        return false;
  return true;
}

// Tuy chon: ma hoa / giai ma chuoi
string ma_hoa(const string &s, const vector<char> &ky_tu,
              const vector<string> &ma) {
  string kq;
  for (char c : s)
    for (size_t i = 0; i < ky_tu.size(); i++)
      if (ky_tu[i] == c) {
        kq += ma[i];
        break;
      }
  return kq;
}

string giai_ma(const string &bits, const vector<Nut> &cay, int goc) {
  string kq;
  if (cay[goc].trai == -1) { // mot ky tu: moi bit la mot ky tu
    for (size_t i = 0; i < bits.size(); i++)
      kq += cay[goc].kt;
    return kq;
  }
  int cur = goc;
  for (char b : bits) {
    cur = (b == '0') ? cay[cur].trai : cay[cur].phai;
    if (cay[cur].trai == -1) {
      kq += cay[cur].kt;
      cur = goc;
    }
  }
  return kq;
}

// Kiem tra hop le bang tan suat theo dieu kien phieu (m trong 1..8, ky tu khong trung, f trong 1..100)
bool kiem_tra_hop_le(int m, const vector<char> &ky_tu, const vector<int> &f) {
  if (m < 1 || m > 8 || (int)ky_tu.size() != m || (int)f.size() != m)
    return false;
  for (int i = 0; i < m; i++) {
    if (f[i] < 1 || f[i] > 100)
      return false;
    for (int j = i + 1; j < m; j++)
      if (ky_tu[i] == ky_tu[j])
        return false;
  }
  return true;
}

// Chay toan bo quy trinh cho mot bang tan suat; tra ve tong bit
long long chay(const vector<char> &ky_tu, const vector<int> &f,
               const string &chuoi_thu = "") {
  vector<Nut> cay;
  dem_so_sanh = 0;
  int goc = tao_cay_huffman(ky_tu, f, cay);
  vector<string> ma(ky_tu.size());
  tao_ma(cay, goc, "", ma);
  long long tong = tong_so_bit(ma, f);

  cout << "Ky tu | Tan suat | Ma | Do dai\n";
  for (size_t i = 0; i < ky_tu.size(); i++)
    cout << "  " << ky_tu[i] << "   |   " << setw(3) << f[i] << "    | "
         << ma[i] << " | " << ma[i].size() << "\n";
  cout << "Tong bit = " << tong << ", trong so tai goc = " << cay[goc].trong_so
       << "\n";
  cout << "So phep so sanh trong luong = " << dem_so_sanh << "\n";
  cout << "Khong ma nao la tien to cua ma khac: "
       << (khong_tien_to(ma) ? "DUNG" : "SAI") << "\n";

  if (!chuoi_thu.empty()) {
    string bits = ma_hoa(chuoi_thu, ky_tu, ma);
    string lai = giai_ma(bits, cay, goc);
    cout << "Chuoi \"" << chuoi_thu << "\" -> " << bits << " (" << bits.size()
         << " bit)\n";
    cout << "Giai ma lai: \"" << lai << "\" -> "
         << (lai == chuoi_thu ? "KHOI PHUC DUNG" : "SAI") << "\n";
  }
  return tong;
}

void chay_kiem_tra() {
  struct Ca {
    vector<char> kt;
    vector<int> f;
    long long mong_doi;
  };
  vector<Ca> cac_ca = {
      {{'A'}, {7}, 7},
      {{'A', 'B'}, {2, 3}, 5},
      {{'A', 'B', 'C'}, {1, 1, 1}, 5},
      {{'A', 'B', 'C'}, {1, 1, 8}, 12},
      {{'A', 'B', 'C', 'D'}, {5, 9, 12, 13}, 78},
  };
  for (size_t i = 0; i < cac_ca.size(); i++) {
    cout << "\n=== Ca kiem tra " << i + 1 << " ===\n";
    long long tong = chay(cac_ca[i].kt, cac_ca[i].f,
                          cac_ca[i].kt.size() > 1 ? "ABBA" : "AAA");
    bool ok = (tong == cac_ca[i].mong_doi);
    cout << "Mong doi " << cac_ca[i].mong_doi << " -> "
         << (ok ? "DAT" : "KHONG DAT") << "\n";
  }

  // Bo kiem thu 12 ca doc lap (tach rieng so ca va so assert)
  struct KqChay {
    long long tong_bit;
    int trong_so_goc;
    long long so_sanh;
    bool prefix_ok;
    vector<string> ma;
    string bits;
    string khoi_phuc;
  };
  auto chay_im = [](const vector<char> &kt, const vector<int> &f,
                    const string &s = "") -> KqChay {
    vector<Nut> cay;
    dem_so_sanh = 0;
    int goc = tao_cay_huffman(kt, f, cay);
    vector<string> ma(kt.size());
    tao_ma(cay, goc, "", ma);
    long long tb = tong_so_bit(ma, f);
    bool pf = khong_tien_to(ma);
    string b = s.empty() ? "" : ma_hoa(s, kt, ma);
    string kp = s.empty() ? "" : giai_ma(b, cay, goc);
    return {tb, cay[goc].trong_so, dem_so_sanh, pf, ma, b, kp};
  };

  int so_ca_pass = 0, tong_so_ca = 12;
  int so_assert_pass = 0, tong_so_assert = 0;
  auto ghi_ca = [&](const string &ma_ca, const string &mo_ta,
                    const vector<bool> &asserts) {
    bool ca_ok = true;
    int pass_cua_ca = 0;
    for (bool a : asserts) {
      tong_so_assert++;
      if (a) {
        so_assert_pass++;
        pass_cua_ca++;
      } else {
        ca_ok = false;
      }
    }
    if (ca_ok)
      so_ca_pass++;
    cout << left << setw(8) << ma_ca << "| " << setw(38) << mo_ta << "| "
         << pass_cua_ca << "/" << asserts.size() << " assert | "
         << (ca_ok ? "PASS" : "FAIL") << "\n";
  };

  cout << "\n================ TONG KET 12 CA KIEM THU DOC LAP ================\n";
  cout << "Ma ca   | Doi tuong va Input                    | Phep kiem  | Ket qua\n";
  cout << "-----------------------------------------------------------------\n";

  // 1. VD01: Vi du 1 phieu & Ca phieu 5
  auto r1 = chay_im({'A', 'B', 'C', 'D'}, {5, 9, 12, 13});
  ghi_ca("VD01", "Vi du 1: A..D, f=[5,9,12,13]",
         {r1.tong_bit == 78, r1.trong_so_goc == 39, r1.prefix_ok});

  // 2. VD02: Vi du 2 phieu & Ca phieu 2
  auto r2 = chay_im({'A', 'B'}, {2, 3}, "ABBA");
  ghi_ca("VD02", "Vi du 2: A B, f=[2,3], s=\"ABBA\"",
         {r2.tong_bit == 5, r2.trong_so_goc == 5, r2.khoi_phuc == "ABBA"});

  // 3. TK01: Tu thiet ke 1 (f khong sap tang + ma hoa/giai ma)
  auto r3 = chay_im({'A', 'B', 'C', 'D'}, {1, 4, 2, 5}, "ACDB");
  ghi_ca("TK01", "Tu TK 1: A..D, f=[1,4,2,5], s=\"ACDB\"",
         {r3.tong_bit == 22, r3.trong_so_goc == 12, r3.bits == "100101011",
          r3.khoi_phuc == "ACDB"});

  // 4. TK02: Tu thiet ke 2 (5 ky tu, nhieu trong so hoa)
  auto r4 = chay_im({'A', 'B', 'C', 'D', 'E'}, {2, 2, 4, 4, 8}, "EDCBA");
  ghi_ca("TK02", "Tu TK 2: A..E, f=[2,2,4,4,8]",
         {r4.tong_bit == 44, r4.trong_so_goc == 20, r4.prefix_ok});

  // 5. BIEN01: Bien m=1 (Ca phieu 1)
  auto r5 = chay_im({'A'}, {7}, "AAA");
  ghi_ca("BIEN01", "Bien m=1: A, f=[7], s=\"AAA\"",
         {r5.ma[0] == "0", r5.tong_bit == 7, r5.so_sanh == 0,
          r5.khoi_phuc == "AAA"});

  // 6. BIEN02: Bien m=8 toi da, tan suat bang nhau
  auto r6 = chay_im({'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'},
                    {5, 5, 5, 5, 5, 5, 5, 5});
  ghi_ca("BIEN02", "Bien m=8: A..H, f=[5..5]",
         {r6.tong_bit == 120, r6.trong_so_goc == 40, r6.prefix_ok});

  // 7. BIEN03: Bien f[i]=1 nho nhat & hoa toan bo (Ca phieu 3)
  auto r7 = chay_im({'A', 'B', 'C'}, {1, 1, 1}, "ABBA");
  ghi_ca("BIEN03", "Bien f=1: A B C, f=[1,1,1]",
         {r7.tong_bit == 5, r7.trong_so_goc == 3, r7.prefix_ok,
          r7.khoi_phuc == "ABBA"});

  // 8. LOI01: Validator tu choi input sai (m ngoai 1..8, trung ky tu, f>100)
  ghi_ca("LOI01", "Validator: m=0, trung KT, f=120>100",
         {!kiem_tra_hop_le(0, {}, {}),
          !kiem_tra_hop_le(2, {'A', 'A'}, {3, 4}),
          !kiem_tra_hop_le(4, {'A', 'B', 'C', 'D'}, {120, 12, 7, 19})});

  // 9. KT09: Ca phieu 4 (tan suat lech manh 1, 1, 8)
  auto r9 = chay_im({'A', 'B', 'C'}, {1, 1, 8});
  ghi_ca("KT09", "Ca phieu 4: A B C, f=[1,1,8]",
         {r9.tong_bit == 12, r9.ma[2].size() == 1, r9.prefix_ok});

  // 10. KT10: Bien tan suat cuc dai f[i]=100 voi m=8
  auto r10 = chay_im({'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'},
                     {100, 100, 100, 100, 100, 100, 100, 100});
  ghi_ca("KT10", "Bien f=100: A..H, f=[100..100]",
         {r10.tong_bit == 2400, r10.trong_so_goc == 800, r10.so_sanh == 49});

  // 11. KT11: Ham con lay_hai_nho_nhat khi hoa trong so
  vector<Nut> cay11 = {{4, 'A', -1, -1},
                       {2, 'B', -1, -1},
                       {2, 'C', -1, -1},
                       {5, 'D', -1, -1}};
  vector<int> ds11 = {0, 1, 2, 3};
  int vt1 = -1, vt2 = -1;
  dem_so_sanh = 0;
  lay_hai_nho_nhat(cay11, ds11, vt1, vt2);
  ghi_ca("KT11", "Ham lay_hai_nho_nhat: hoa [4,2,2,5]",
         {vt1 == 1, vt2 == 2, dem_so_sanh == 5});

  // 12. KT12: Ham con khong_tien_to (ca am tinh va duong tinh)
  ghi_ca("KT12", "Ham khong_tien_to: bo ma sai & dung",
         {!khong_tien_to({"0", "01", "11"}),
          khong_tien_to({"0", "10", "11"})});

  cout << "-----------------------------------------------------------------\n";
  cout << "TONG KET KIEM THU:\n";
  cout << "  - So ca kiem thu doc lap : " << so_ca_pass << "/" << tong_so_ca
       << " PASS (" << (tong_so_ca - so_ca_pass) << " FAIL)\n";
  cout << "  - Tong so phep kiem (assert): " << so_assert_pass << "/"
       << tong_so_assert << " PASS (" << (tong_so_assert - so_assert_pass)
       << " FAIL)\n";
}

void nhap_tu_ban_phim() {
  int m;
  while (true) {
    cout << "Nhap so ky tu (1..8): ";
    if (!(cin >> m)) {
      if (cin.eof())
        return;
      cin.clear();
      m = 0;
    }
    cin.ignore(1000, '\n');
    if (m >= 1 && m <= 8)
      break;
    cout << "  So ky tu phai la so nguyen tu 1 den 8. Vui long nhap lai!\n";
  }

  vector<char> ky_tu;
  vector<int> f;

  // 1. Nhap cac ky tu tren cung 1 hang
  string line;
  while (true) {
    cout << "Nhap " << m << " ky tu: ";
    if (!getline(cin, line))
      return;
    if (line.empty())
      continue;
    stringstream ss(line);
    vector<char> temp;
    char c;
    while (ss >> c) {
      temp.push_back(c);
    }
    if ((int)temp.size() != m) {
      cout << "  So luong ky tu chua dung (da nhap " << temp.size()
            << ", yeu cau " << m << "). Vui long nhap lai!\n";
      continue;
    }
    bool trung = false;
    for (int i = 0; i < m; i++) {
      for (int j = i + 1; j < m; j++) {
        if (temp[i] == temp[j]) {
          trung = true;
          break;
        }
      }
      if (trung)
        break;
    }
    if (trung) {
      cout << "  Co ky tu bi trung lap. Vui long nhap lai!\n";
      continue;
    }
    ky_tu = temp;
    break;
  }

  // 2. Nhap tan suat tren cung 1 hang
  while (true) {
    cout << "Nhap " << m << " tan suat tuong ung: ";
    if (!getline(cin, line))
      return;
    if (line.empty())
      continue;
    stringstream ss(line);
    vector<int> temp;
    int val;
    bool loi = false;
    while (ss >> val) {
      if (val < 1 || val > 100) {
        loi = true;
        break;
      }
      temp.push_back(val);
    }
    string thua;
    if (ss >> thua || loi || (int)temp.size() != m) {
      cout << "  Tan suat phai gom dung " << m
           << " so nguyen trong khoang 1..100. Vui long nhap lai!\n";
      continue;
    }
    f = temp;
    break;
  }

  // 3. Chuoi de ma hoa / giai ma thu (tuy chon)
  string s;
  cout << "Nhap chuoi ngan de ma hoa/giai ma thu (chi gom cac ky tu tren, "
          "Enter de bo qua): ";
  getline(cin, s);
  bool hop_le = true;
  for (char c : s) {
    bool co = false;
    for (char x : ky_tu)
      if (x == c)
        co = true;
    if (!co)
      hop_le = false;
  }
  if (!hop_le) {
    cout << "Chuoi co ky tu ngoai bang, bo qua ma hoa.\n";
    s = "";
  }
  cout << "\n";
  chay(ky_tu, f, s);
}

int main() {
  int chon;
  do {
    cout << "\n MA HUFFMAN\n"
         << "1. Chay cac ca kiem tra (gom 2 vi du de bai)\n"
         << "2. Nhap bang tan suat tu ban phim\n"
         << "0. Thoat\n"
         << "Chon: ";
    if (!(cin >> chon)) {
      if (cin.eof())
        break;
      cin.clear();
      cin.ignore(1000, '\n');
      cout << "  Lua chon khong hop le, vui long nhap 0, 1 hoac 2.\n";
      chon = -1;
      continue;
    }
    cin.ignore(1000, '\n'); // bo phan con lai cua dong (vd: go "1 2" chi lay 1)
    if (chon == 1)
      chay_kiem_tra();
    else if (chon == 2)
      nhap_tu_ban_phim();
    else if (chon != 0)
      cout << "  Lua chon khong hop le, vui long nhap 0, 1 hoac 2.\n";
  } while (chon != 0);
  return 0;
}
