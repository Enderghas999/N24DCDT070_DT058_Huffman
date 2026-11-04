// ============================================================================
//  DT058 - Tao ma Huffman tu bang tan suat nho (INT13185)
//  Web truc quan hoa va danh gia thuat toan tham lam - may chu C++17.
//
//  Sinh vien : Dang The Phu - N24DCDT070 - Lop D24CQDT01-N - Nhom 03
//
//  API (tra ve JSON):
//    GET  /api/chay?kt=A B C D&f=5 9 12 13&s=ABCD&che_do=hoc
//         Kiem tra dau vao, chay thuat toan, tra ve cay, bang ma, cac bo dem,
//         thoi gian (trung vi) va danh sach su kien mo phong; luu lich su.
//    GET  /api/danh_gia?m=2,4,8,16&lap=10&hat=1
//         Thuc nghiem: do thoi gian va bo dem theo m (bang tan suat ngau nhien).
//    GET  /api/kiem_tra?n=2000&hat=2026   Chay bo kiem tra tinh dung.
//    GET  /api/lich_su?gioi_han=20         Lich su cac lan chay.
//    GET  /api/moi_truong                  Thong tin moi truong do.
//  Trang web tinh nam trong thu muc public/ (bien PUBLIC_DIR de doi).
//
//  Luu lich su:
//    - Mac dinh: tep JSON Lines (bien HISTORY_FILE, mac dinh data/lich_su.jsonl),
//      ben vung qua cac lan khoi dong lai tren cung mot may.
//    - Khi bien dich voi -DUSE_PG va dat bien DATABASE_URL: luu vao PostgreSQL
//      (dung khi trien khai len Render, vi o dia cua web Free la tam thoi).
//
//  Bien dich (Windows/MinGW):
//    g++ -std=c++17 -O2 server.cpp -o huffman_app.exe -lws2_32
//  Bien dich (Linux):  g++ -std=c++17 -O2 server.cpp -o huffman_app -lpthread
//  Co PostgreSQL:      them -DUSE_PG -lpq
// ============================================================================
#ifndef CPPHTTPLIB_NO_EXCEPTIONS
#define CPPHTTPLIB_NO_EXCEPTIONS 1
#endif
#include "httplib.h"

#include "bo_kiem_tra.hpp"
#include "huffman_core.hpp"
#include "upstash_cache.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <sstream>
#include <string>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif
#include <thread>
#include <vector>

#ifdef USE_PG
#include <libpq-fe.h>
#endif

using namespace std;
using hf::json_str;

// ============================================================================
//  1. Do thoi gian: lap `lap` mau, moi mau chay `lo` lan lien tiep de vuot do
//     phan giai dong ho; tra ve trung vi, nho nhat, lon nhat (ns / lan chay).
// ============================================================================
struct DoThoiGian {
  double trung_vi = 0, nho_nhat = 0, lon_nhat = 0;
  long long lo = 1;
  int lap = 0;
};

static volatile long long g_chong_toi_uu = 0; // giu ket qua de trinh bien dich
                                              // khong bo phep tinh

template <class F> static DoThoiGian do_thoi_gian(F &&viec, int lap) {
  using clk = chrono::steady_clock;
  // Chon so lan chay moi mau sao cho mot mau keo dai >= 50 micro giay
  long long lo = 1;
  for (;;) {
    auto t0 = clk::now();
    for (long long r = 0; r < lo; r++)
      g_chong_toi_uu = g_chong_toi_uu + viec();
    double ns = chrono::duration<double, nano>(clk::now() - t0).count();
    if (ns >= 50000 || lo >= (1LL << 20))
      break;
    lo *= 2;
  }
  vector<double> mau;
  for (int k = 0; k < lap; k++) {
    auto t0 = clk::now();
    for (long long r = 0; r < lo; r++)
      g_chong_toi_uu = g_chong_toi_uu + viec();
    mau.push_back(chrono::duration<double, nano>(clk::now() - t0).count() /
                  (double)lo);
  }
  sort(mau.begin(), mau.end());
  DoThoiGian d;
  d.lap = lap;
  d.lo = lo;
  d.nho_nhat = mau.front();
  d.lon_nhat = mau.back();
  d.trung_vi = (lap % 2) ? mau[lap / 2] : (mau[lap / 2 - 1] + mau[lap / 2]) / 2;
  return d;
}

static string json_do(const DoThoiGian &d) {
  ostringstream o;
  o.setf(ios::fixed);
  o.precision(1);
  o << "{\"trung_vi_ns\":" << d.trung_vi << ",\"nho_nhat_ns\":" << d.nho_nhat
    << ",\"lon_nhat_ns\":" << d.lon_nhat << ",\"lap\":" << d.lap
    << ",\"lo\":" << d.lo << "}";
  return o.str();
}

// Nhiem vu 1: chi tinh tong bit (dung cay, cong trong so cac nut cha)
static long long nv_tinh_tong_bit(const vector<int> &f) {
  vector<hf::Nut> cay;
  long long dem = 0, tong = 0;
  hf::tao_cay_huffman(f, cay, dem, tong);
  return f.size() == 1 ? f[0] : tong;
}
// Nhiem vu 2: sinh day du bang ma (dung cay + duyet cay tao ma + tong bit)
static long long nv_sinh_bang_ma(const vector<int> &f) {
  hf::KetQua r = hf::chay(f);
  return r.tong_bit + (long long)r.ma.back().size();
}

static string thoi_diem_utc() {
  time_t t = time(nullptr);
  tm g{};
#ifdef _WIN32
  gmtime_s(&g, &t);
#else
  gmtime_r(&t, &g);
#endif
  char b[32];
  strftime(b, sizeof b, "%Y-%m-%dT%H:%M:%SZ", &g);
  return b;
}

static string moi_truong_json(const string &kieu_luu) {
  ostringstream o;
#if defined(_WIN32)
  const char *hdh = "Windows";
#elif defined(__linux__)
  const char *hdh = "Linux";
#elif defined(__APPLE__)
  const char *hdh = "macOS";
#else
  const char *hdh = "khac";
#endif
#ifdef __OPTIMIZE__
  const char *toi_uu = "bật (-O1 trở lên)";
#else
  const char *toi_uu = "tắt (-O0)";
#endif
#ifdef __VERSION__
  string trinh = string("g++ ") + __VERSION__;
#else
  string trinh = "không rõ";
#endif
  o << "{\"he_dieu_hanh\":" << json_str(hdh)
    << ",\"trinh_bien_dich\":" << json_str(trinh)
    << ",\"toi_uu\":" << json_str(toi_uu)
    << ",\"so_luong_cpu\":" << thread::hardware_concurrency()
    << ",\"dong_ho\":\"std::chrono::steady_clock\""
    << ",\"luu_lich_su\":" << json_str(kieu_luu)
    << ",\"cache_redis\":" << json_str(uc::bat() ? "BAT" : "TAT") << "}";
  return o.str();
}

// ============================================================================
//  2. Luu lich su: tep JSON Lines hoac PostgreSQL
// ============================================================================
struct LuuTru {
  virtual ~LuuTru() = default;
  virtual string ten() const = 0;
  // du_lieu: doi tuong JSON (khong co id/thoi_diem); tra ve id, -1 neu loi
  virtual long long them(const string &du_lieu) = 0;
  virtual string danh_sach(int gioi_han) = 0; // mang JSON, moi nhat truoc
};

// Ghep id va thoi diem vao dau doi tuong JSON du lieu
static string ghep_ban_ghi(long long id, const string &thoi_diem,
                           const string &du_lieu) {
  return "{\"id\":" + to_string(id) + ",\"thoi_diem\":" + json_str(thoi_diem) +
         "," + du_lieu.substr(1);
}

struct LuuTep : LuuTru {
  string duong_dan;
  mutex khoa;
  long long id_cuoi = 0;
  explicit LuuTep(string p) : duong_dan(std::move(p)) {
    size_t vt = duong_dan.find_last_of("/\\");
    if (vt != string::npos) {
#ifdef _WIN32
      _mkdir(duong_dan.substr(0, vt).c_str());
#else
      mkdir(duong_dan.substr(0, vt).c_str(), 0755);
#endif
    }
    ifstream in(duong_dan);
    string dong;
    while (getline(in, dong)) {
      size_t a = dong.find("\"id\":");
      if (a != string::npos)
        id_cuoi = max(id_cuoi, atoll(dong.c_str() + a + 5));
    }
  }
  string ten() const override { return "tệp " + duong_dan; }
  long long them(const string &du_lieu) override {
    lock_guard<mutex> k(khoa);
    long long id = ++id_cuoi;
    ofstream out(duong_dan, ios::app);
    if (!out)
      return -1;
    out << ghep_ban_ghi(id, thoi_diem_utc(), du_lieu) << "\n";
    return id;
  }
  string danh_sach(int gioi_han) override {
    lock_guard<mutex> k(khoa);
    ifstream in(duong_dan);
    vector<string> dong;
    string s;
    while (getline(in, s))
      if (!s.empty())
        dong.push_back(s);
    string o = "[";
    int n = 0;
    for (int i = (int)dong.size() - 1; i >= 0 && n < gioi_han; i--, n++)
      o += (n ? "," : "") + dong[i];
    return o + "]";
  }
};

#ifdef USE_PG
struct LuuPg : LuuTru {
  string url;
  PGconn *ket_noi = nullptr;
  mutex khoa;
  explicit LuuPg(string u) : url(std::move(u)) {
    lock_guard<mutex> k(khoa);
    if (dam_bao_ket_noi()) {
      PGresult *r = PQexec(ket_noi,
                           "CREATE TABLE IF NOT EXISTS lich_su ("
                           " id BIGSERIAL PRIMARY KEY,"
                           " thoi_diem TIMESTAMPTZ NOT NULL DEFAULT now(),"
                           " du_lieu JSONB NOT NULL)");
      if (PQresultStatus(r) != PGRES_COMMAND_OK)
        cerr << "PostgreSQL: khong tao duoc bang: " << PQerrorMessage(ket_noi);
      PQclear(r);
    }
  }
  ~LuuPg() override {
    if (ket_noi)
      PQfinish(ket_noi);
  }
  bool dam_bao_ket_noi() {
    if (ket_noi && PQstatus(ket_noi) == CONNECTION_OK)
      return true;
    if (ket_noi) {
      PQreset(ket_noi);
      if (PQstatus(ket_noi) == CONNECTION_OK)
        return true;
      PQfinish(ket_noi);
    }
    ket_noi = PQconnectdb(url.c_str());
    if (PQstatus(ket_noi) != CONNECTION_OK) {
      cerr << "PostgreSQL: ket noi that bai: " << PQerrorMessage(ket_noi);
      return false;
    }
    return true;
  }
  string ten() const override { return "PostgreSQL"; }
  long long them(const string &du_lieu) override {
    lock_guard<mutex> k(khoa);
    if (!dam_bao_ket_noi())
      return -1;
    const char *ts[1] = {du_lieu.c_str()};
    PGresult *r = PQexecParams(
        ket_noi, "INSERT INTO lich_su (du_lieu) VALUES ($1::jsonb) RETURNING id",
        1, nullptr, ts, nullptr, nullptr, 0);
    long long id = -1;
    if (PQresultStatus(r) == PGRES_TUPLES_OK && PQntuples(r) == 1)
      id = atoll(PQgetvalue(r, 0, 0));
    else
      cerr << "PostgreSQL: ghi that bai: " << PQerrorMessage(ket_noi);
    PQclear(r);
    return id;
  }
  string danh_sach(int gioi_han) override {
    lock_guard<mutex> k(khoa);
    if (!dam_bao_ket_noi())
      return "[]";
    string n = to_string(gioi_han);
    const char *ts[1] = {n.c_str()};
    PGresult *r = PQexecParams(
        ket_noi,
        "SELECT id, to_char(thoi_diem AT TIME ZONE 'UTC', "
        "'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"'), du_lieu::text FROM lich_su "
        "ORDER BY id DESC LIMIT $1::int",
        1, nullptr, ts, nullptr, nullptr, 0);
    string o = "[";
    if (PQresultStatus(r) == PGRES_TUPLES_OK)
      for (int i = 0; i < PQntuples(r); i++)
        o += (i ? "," : "") + ghep_ban_ghi(atoll(PQgetvalue(r, i, 0)),
                                           PQgetvalue(r, i, 1),
                                           PQgetvalue(r, i, 2));
    PQclear(r);
    return o + "]";
  }
};
#endif

static unique_ptr<LuuTru> tao_luu_tru() {
  const char *db = getenv("DATABASE_URL");
#ifdef USE_PG
  if (db && *db)
    return make_unique<LuuPg>(db);
#else
  if (db && *db)
    cerr << "Canh bao: co DATABASE_URL nhung chuong trinh bien dich khong co "
            "-DUSE_PG; dung tep thay the.\n";
#endif
  const char *p = getenv("HISTORY_FILE");
  return make_unique<LuuTep>(p && *p ? p : "data/lich_su.jsonl");
}

// ============================================================================
//  3. Xu ly API
// ============================================================================
static string loi_json(const string &msg) {
  return "{\"loi\":" + json_str(msg) + "}";
}

static string bang_noi(const vector<int> &v) {
  string s;
  for (size_t i = 0; i < v.size(); i++)
    s += (i ? " " : "") + to_string(v[i]);
  return s;
}

// Ghep phan hoi: phan "rest" (da co the lay tu Upstash) + cac truong luon moi
static string ghep_phan_hoi(long long id, const string &tt, double ms,
                            const string &rest) {
  ostringstream o;
  o.setf(ios::fixed);
  o.precision(1);
  o << "{\"id_lich_su\":" << id << ",\"cache\":\"" << tt
    << "\",\"xu_ly_server_ms\":" << ms << "," << rest;
  return o.str();
}

static string xu_ly_chay(const httplib::Request &req, LuuTru &luu, int &status) {
  auto t_bat_dau = chrono::steady_clock::now();
  string kt_raw = req.get_param_value("kt"), f_raw = req.get_param_value("f"),
         s = req.get_param_value("s"), che_do = req.get_param_value("che_do");
  if (che_do != "tinh" && che_do != "hoc" && che_do != "luyen")
    che_do = "hoc";
  vector<char> kt;
  vector<int> f;
  string loi = hf::doc_dau_vao(kt_raw, f_raw, s, kt, f);
  if (!loi.empty()) {
    status = 400;
    return loi_json(loi);
  }
  const int m = (int)f.size();

  // (0) Bo nho dem Upstash Redis: cung dau vao thi lay ket qua da tinh
  string tt = "off";
  string khoa_cache;
  if (uc::bat()) {
    khoa_cache = uc::rut_gon("huffman:v2:" + che_do + "|" +
                             string(kt.begin(), kt.end()) + "|" + bang_noi(f) +
                             "|" + s);
    string gt;
    int kq = uc::lay(khoa_cache, gt);
    size_t cat_vt = gt.find('\n');
    if (kq == 1 && cat_vt != string::npos) {
      long long id_hit = luu.them(gt.substr(0, cat_vt)); // van ghi lich su
      double ms_hit = chrono::duration<double, milli>(
                          chrono::steady_clock::now() - t_bat_dau)
                          .count();
      cout << "[Upstash] HIT  " << khoa_cache << endl;
      return ghep_phan_hoi(id_hit, "HIT", ms_hit, gt.substr(cat_vt + 1));
    }
    tt = (kq == 0 || kq == 1) ? "MISS" : "loi";
  }

  // (a) Chay co ghi vet cho mo phong
  hf::GhiVet gv;
  for (char c : kt)
    gv.nhan.push_back(string(1, c));
  gv.ma.assign(m, "");
  auto t0 = chrono::steady_clock::now();
  hf::KetQua r = hf::chay(f, &gv);
  {
    string loi_tong = "Tổng số bit = ";
    for (int i = 0; i < m; i++)
      loi_tong += (i ? " + " : "") + to_string(f[i]) + "×" +
                  to_string(r.ma[i].size());
    loi_tong += " = " + to_string(r.tong_bit) + ".";
    if (m > 1)
      loi_tong += " Bằng tổng trọng số các nút cha (" + to_string(r.tong_noi) +
                  "), vì mỗi lần ghép làm mọi lá bên dưới dài thêm 1 bit.";
    gv.ngan_xep.clear();
    gv.ghi("tong", 15, loi_tong);
  }
  double ghi_vet_ns =
      chrono::duration<double, nano>(chrono::steady_clock::now() - t0).count();

  // (b) Do thoi gian thuat toan thuan (khong ghi vet), 10 mau, lay trung vi
  DoThoiGian d1 = do_thoi_gian([&] { return nv_tinh_tong_bit(f); }, 10);
  DoThoiGian d2 = do_thoi_gian([&] { return nv_sinh_bang_ma(f); }, 10);

  string bits = s.empty() ? "" : hf::ma_hoa(s, kt, r.ma);
  string lai = s.empty() ? "" : hf::giai_ma(bits, r.cay, r.goc, kt);
  long long khoi_luong = 0;
  for (auto &x : r.ma)
    khoi_luong += (long long)x.size();

  // (c) Luu lich su (tham so + thong ke, khong luu vet)
  ostringstream ls;
  ls << "{\"kt\":" << json_str(kt_raw) << ",\"f\":" << json_str(bang_noi(f))
     << ",\"s\":" << json_str(s) << ",\"che_do\":" << json_str(che_do)
     << ",\"thuat_toan\":\"tham lam, quét tuyến tính\",\"m\":" << m
     << ",\"tong_bit\":" << r.tong_bit << ",\"so_sanh\":" << r.so_sanh
     << ",\"so_lan_ghep\":" << (m - 1) << ",\"so_loi_goi\":" << r.so_loi_goi
     << ",\"t_sinh_ma_ns\":" << (long long)(d2.trung_vi + 0.5) << "}";
  long long id = luu.them(ls.str());

  ostringstream o;
  o.setf(ios::fixed);
  o.precision(1);
  o << "\"m\":" << m << ",\"kt\":[";
  for (int i = 0; i < m; i++)
    o << (i ? "," : "") << json_str(string(1, kt[i]));
  o << "],\"f\":" << hf::json_ints(f) << ",\"s\":" << json_str(s)
    << ",\"che_do\":" << json_str(che_do) << ",\"cay\":[";
  for (size_t i = 0; i < r.cay.size(); i++)
    o << (i ? "," : "") << "{\"w\":" << r.cay[i].trong_so
      << ",\"la\":" << r.cay[i].la << ",\"trai\":" << r.cay[i].trai
      << ",\"phai\":" << r.cay[i].phai << "}";
  o << "],\"goc\":" << r.goc << ",\"ma\":[";
  for (int i = 0; i < m; i++)
    o << (i ? "," : "") << json_str(r.ma[i]);
  o << "],\"tong_bit\":" << r.tong_bit << ",\"tong_noi\":" << r.tong_noi
    << ",\"khong_tien_to\":" << (hf::khong_tien_to(r.ma) ? "true" : "false")
    << ",\"bits\":" << json_str(bits) << ",\"giai_ma\":" << json_str(lai)
    << ",\"dem\":{\"so_sanh\":" << r.so_sanh
    << ",\"cong_thuc_so_sanh\":" << (long long)(m - 1) * (m - 1)
    << ",\"so_lan_ghep\":" << (m - 1) << ",\"so_nut\":" << r.cay.size()
    << ",\"so_loi_goi\":" << r.so_loi_goi
    << ",\"khoi_luong_dau_ra\":" << khoi_luong << "}"
    << ",\"thoi_gian\":{\"tinh_tong_bit\":" << json_do(d1)
    << ",\"sinh_bang_ma\":" << json_do(d2) << ",\"ghi_vet_ns\":" << ghi_vet_ns
    << "},\"bi_cat\":" << (gv.bi_cat ? "true" : "false") << ",\"su_kien\":[";
  for (size_t i = 0; i < gv.su_kien.size(); i++)
    o << (i ? "," : "") << gv.su_kien[i];
  o << "]}";
  string rest = o.str();
  if (uc::bat()) {
    if (tt == "MISS" && !uc::dat(khoa_cache, ls.str() + "\n" + rest, 3600))
      tt = "loi";
    cout << "[Upstash] " << (tt == "MISS" ? "MISS " : "LOI  ") << khoa_cache
         << endl;
  }
  double xu_ly_ms = chrono::duration<double, milli>(chrono::steady_clock::now() -
                                                    t_bat_dau)
                        .count();
  return ghep_phan_hoi(id, tt, xu_ly_ms, rest);
}

static string xu_ly_danh_gia(const httplib::Request &req, int &status) {
  string ms = req.has_param("m") ? req.get_param_value("m")
                                 : "2,4,8,16,32,64,128,256,512,1024";
  int lap = req.has_param("lap") ? atoi(req.get_param_value("lap").c_str()) : 10;
  unsigned hat = req.has_param("hat")
                     ? (unsigned)atoi(req.get_param_value("hat").c_str())
                     : 1u;
  lap = max(1, min(lap, 30));
  vector<int> ds_m;
  {
    string tok;
    istringstream is(ms);
    while (getline(is, tok, ',')) {
      int v = atoi(tok.c_str());
      if (v < 1 || v > 4096) {
        status = 400;
        return loi_json("Mỗi giá trị m phải nằm trong 1..4096 (nhận \"" + tok +
                        "\").");
      }
      ds_m.push_back(v);
    }
  }
  if (ds_m.empty() || ds_m.size() > 16) {
    status = 400;
    return loi_json("Cần từ 1 đến 16 giá trị m, cách nhau bằng dấu phẩy.");
  }
  ostringstream o;
  o.setf(ios::fixed);
  o.precision(1);
  o << "{\"lap\":" << lap << ",\"hat\":" << hat << ",\"ket_qua\":[";
  for (size_t k = 0; k < ds_m.size(); k++) {
    int m = ds_m[k];
    mt19937 rng(hat * 1000003u + (unsigned)m);
    vector<int> f(m);
    for (int &v : f)
      v = 1 + (int)(rng() % 100);
    hf::KetQua r = hf::chay(f);
    long long kl = 0;
    int dai_nhat = 0;
    for (auto &x : r.ma) {
      kl += (long long)x.size();
      dai_nhat = max(dai_nhat, (int)x.size());
    }
    DoThoiGian d1 = do_thoi_gian([&] { return nv_tinh_tong_bit(f); }, lap);
    DoThoiGian d2 = do_thoi_gian([&] { return nv_sinh_bang_ma(f); }, lap);
    o << (k ? "," : "") << "{\"m\":" << m << ",\"tong_bit\":" << r.tong_bit
      << ",\"so_sanh\":" << r.so_sanh
      << ",\"cong_thuc\":" << (long long)(m - 1) * (m - 1)
      << ",\"so_loi_goi\":" << r.so_loi_goi << ",\"khoi_luong\":" << kl
      << ",\"ma_dai_nhat\":" << dai_nhat
      << ",\"tinh_tong_bit\":" << json_do(d1)
      << ",\"sinh_bang_ma\":" << json_do(d2) << "}";
  }
  o << "]}";
  return o.str();
}

static string xu_ly_kiem_tra(const httplib::Request &req) {
  int n = req.has_param("n") ? atoi(req.get_param_value("n").c_str()) : 2000;
  unsigned hat = req.has_param("hat")
                     ? (unsigned)atoi(req.get_param_value("hat").c_str())
                     : 2026u;
  n = max(1, min(n, 50000));
  auto t0 = chrono::steady_clock::now();
  auto cac = hf::chay_bo_kiem_tra(n, hat);
  double ms =
      chrono::duration<double, milli>(chrono::steady_clock::now() - t0).count();
  int ca_dat = 0, pk = 0, pk_dat = 0;
  ostringstream o;
  o << "{\"ca\":[";
  for (size_t i = 0; i < cac.size(); i++) {
    auto &c = cac[i];
    ca_dat += c.dat();
    o << (i ? "," : "") << "{\"ma\":" << json_str(c.ma_ca)
      << ",\"nhom\":" << json_str(c.nhom) << ",\"mo_ta\":" << json_str(c.mo_ta)
      << ",\"dat\":" << (c.dat() ? "true" : "false") << ",\"kiem\":[";
    for (size_t j = 0; j < c.kiem.size(); j++) {
      pk++;
      pk_dat += c.kiem[j].dat;
      o << (j ? "," : "") << "{\"mo_ta\":" << json_str(c.kiem[j].mo_ta)
        << ",\"dat\":" << (c.kiem[j].dat ? "true" : "false")
        << ",\"chi_tiet\":" << json_str(c.kiem[j].chi_tiet) << "}";
    }
    o << "]}";
  }
  o << "],\"ca_dat\":" << ca_dat << ",\"ca_tong\":" << cac.size()
    << ",\"pk_dat\":" << pk_dat << ",\"pk_tong\":" << pk << ",\"n\":" << n
    << ",\"hat\":" << hat << ",\"thoi_gian_ms\":" << ms << "}";
  return o.str();
}

// ============================================================================
//  4. main
// ============================================================================
int main() {
  unique_ptr<LuuTru> luu = tao_luu_tru();
  httplib::Server svr;

  const char *pd = getenv("PUBLIC_DIR");
  string thu_muc = pd && *pd ? pd : "./public";
  if (!svr.set_mount_point("/", thu_muc)) {
    cerr << "Loi: khong tim thay thu muc giao dien \"" << thu_muc
         << "\". Hay chay chuong trinh tu thu muc web/.\n";
    return 1;
  }

  auto gui = [](httplib::Response &res, const string &body, int status = 200) {
    res.status = status;
    res.set_header("Cache-Control", "no-store");
    res.set_content(body, "application/json; charset=utf-8");
  };

  svr.Get("/api/chay", [&](const httplib::Request &q, httplib::Response &res) {
    int st = 200;
    string b = xu_ly_chay(q, *luu, st);
    gui(res, b, st);
  });
  svr.Get("/api/danh_gia",
          [&](const httplib::Request &q, httplib::Response &res) {
            int st = 200;
            string b = xu_ly_danh_gia(q, st);
            gui(res, b, st);
          });
  svr.Get("/api/kiem_tra",
          [&](const httplib::Request &q, httplib::Response &res) {
            gui(res, xu_ly_kiem_tra(q));
          });
  svr.Get("/api/lich_su", [&](const httplib::Request &q, httplib::Response &res) {
    int n = q.has_param("gioi_han") ? atoi(q.get_param_value("gioi_han").c_str())
                                    : 20;
    gui(res, luu->danh_sach(max(1, min(n, 200))));
  });
  svr.Get("/api/moi_truong",
          [&](const httplib::Request &, httplib::Response &res) {
            gui(res, moi_truong_json(luu->ten()));
          });

  const char *ps = getenv("PORT");
  int cong = ps && *ps ? atoi(ps) : 8080;
  cout << "=================================================\n"
       << "  DT058 - Ma Huffman (INT13185) - Dang The Phu - N24DCDT070\n"
       << "  Mo trinh duyet: http://localhost:" << cong << "\n"
       << "  Luu lich su   : " << luu->ten() << "\n"
       << "  Upstash Redis (bo nho dem): " << (uc::bat() ? "BAT" : "TAT")
       << "\n"
       << "  Nhan Ctrl+C de dung.\n"
       << "=================================================\n";
  if (!svr.listen("0.0.0.0", cong)) {
    cerr << "Loi: khong mo duoc cong " << cong
         << " (co the chuong trinh khac dang dung).\n";
    return 1;
  }
  return 0;
}
