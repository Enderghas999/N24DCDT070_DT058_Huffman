// ============================================================================
//  upstash_cache.hpp - Bo nho dem phan tan bang Upstash Redis (REST API)
//  DT058 - Dang The Phu - N24DCDT070
//
//  Cach hoat dong: goi REST API cua Upstash bang lenh `curl` (khong can OpenSSL
//  trong C++). Moi lenh Redis la mot mang JSON, vi du ["GET","khoa"].
//  Cau hinh (uu tien theo thu tu):
//    1. Bien moi truong UPSTASH_REDIS_REST_URL va UPSTASH_REDIS_REST_TOKEN
//       (dung khi trien khai tren Render).
//    2. Tep upstash.txt (hoac duong dan trong bien UPSTASH_FILE) o thu muc
//       dang chay: dong 1 = URL, dong 2 = token. KHONG dua tep nay len GitHub.
//  Neu khong co cau hinh, hoac Upstash loi/ngat mang: tat bo nho dem, web van
//  chay binh thuong (chi tinh lai moi lan).
// ============================================================================
#pragma once
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>

namespace uc {

struct CauHinh {
  std::string url, token;
  bool bat = false;
};

inline std::string cat(std::string s) {
  if (s.size() >= 3 && (unsigned char)s[0] == 0xEF && (unsigned char)s[1] == 0xBB &&
      (unsigned char)s[2] == 0xBF)
    s.erase(0, 3); // dau BOM UTF-8
  const char *bo = " \t\r\n\"'";
  size_t a = s.find_first_not_of(bo);
  if (a == std::string::npos)
    return "";
  size_t b = s.find_last_not_of(bo);
  return s.substr(a, b - a + 1);
}

inline bool an_toan(const std::string &s) { // chong chen lenh vao dong lenh curl
  if (s.empty())
    return false;
  for (unsigned char c : s)
    if (!(isalnum(c) || c == '-' || c == '.' || c == '_' || c == '~' ||
          c == ':' || c == '/' || c == '=' || c == '+'))
      return false;
  return true;
}

inline CauHinh doc_cau_hinh() {
  CauHinh c;
  const char *u = getenv("UPSTASH_REDIS_REST_URL");
  const char *t = getenv("UPSTASH_REDIS_REST_TOKEN");
  if (u && t && *u && *t) {
    c.url = cat(u);
    c.token = cat(t);
  } else {
    const char *p = getenv("UPSTASH_FILE");
    std::ifstream in(p && *p ? p : "upstash.txt");
    std::string dong;
    int vt = 0;
    while (std::getline(in, dong)) {
      dong = cat(dong);
      if (dong.empty())
        continue;
      size_t eq = dong.find('=');
      if (dong.compare(0, 7, "UPSTASH") == 0 && eq != std::string::npos) {
        std::string k = cat(dong.substr(0, eq)), v = cat(dong.substr(eq + 1));
        if (k == "UPSTASH_REDIS_REST_URL")
          c.url = v;
        else if (k == "UPSTASH_REDIS_REST_TOKEN")
          c.token = v;
        continue;
      }
      if (vt == 0)
        c.url = dong;
      else if (vt == 1)
        c.token = dong;
      vt++;
    }
  }
  while (!c.url.empty() && c.url.back() == '/')
    c.url.pop_back();
  bool http_ok = c.url.compare(0, 8, "https://") == 0 ||
                 c.url.compare(0, 7, "http://") == 0;
  c.bat = http_ok && an_toan(c.url) && an_toan(c.token);
  return c;
}

inline const CauHinh &cau_hinh() {
  static const CauHinh c = doc_cau_hinh();
  return c;
}
inline bool bat() { return cau_hinh().bat; }

inline std::string json_chuoi(const std::string &s) {
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

inline void utf8_them(std::string &o, unsigned cp) {
  if (cp < 0x80)
    o += (char)cp;
  else if (cp < 0x800) {
    o += (char)(0xC0 | (cp >> 6));
    o += (char)(0x80 | (cp & 0x3F));
  } else if (cp < 0x10000) {
    o += (char)(0xE0 | (cp >> 12));
    o += (char)(0x80 | ((cp >> 6) & 0x3F));
    o += (char)(0x80 | (cp & 0x3F));
  } else {
    o += (char)(0xF0 | (cp >> 18));
    o += (char)(0x80 | ((cp >> 12) & 0x3F));
    o += (char)(0x80 | ((cp >> 6) & 0x3F));
    o += (char)(0x80 | (cp & 0x3F));
  }
}

// Doc chuoi JSON bat dau tai s[i] (s[i] la dau "), tra ve false neu sai
inline bool doc_chuoi_json(const std::string &s, size_t i, std::string &out) {
  out.clear();
  if (i >= s.size() || s[i] != '"')
    return false;
  for (i++; i < s.size(); i++) {
    char c = s[i];
    if (c == '"')
      return true;
    if (c != '\\') {
      out += c;
      continue;
    }
    if (++i >= s.size())
      return false;
    switch (s[i]) {
    case 'n': out += '\n'; break;
    case 'r': out += '\r'; break;
    case 't': out += '\t'; break;
    case 'b': out += '\b'; break;
    case 'f': out += '\f'; break;
    case 'u': {
      auto hex4 = [&](size_t p, unsigned &v) {
        if (p + 4 > s.size())
          return false;
        v = 0;
        for (int k = 0; k < 4; k++) {
          char h = s[p + k];
          v <<= 4;
          if (h >= '0' && h <= '9') v |= h - '0';
          else if (h >= 'a' && h <= 'f') v |= h - 'a' + 10;
          else if (h >= 'A' && h <= 'F') v |= h - 'A' + 10;
          else return false;
        }
        return true;
      };
      unsigned cp;
      if (!hex4(i + 1, cp))
        return false;
      i += 4;
      if (cp >= 0xD800 && cp < 0xDC00 && i + 6 < s.size() && s[i + 1] == '\\' &&
          s[i + 2] == 'u') {
        unsigned lo;
        if (hex4(i + 3, lo) && lo >= 0xDC00 && lo < 0xE000) {
          cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
          i += 6;
        }
      }
      utf8_them(out, cp);
      break;
    }
    default: out += s[i]; // \" \\ \/
    }
  }
  return false;
}

// Gui mot lenh Redis (mang JSON) qua REST; tra ve false neu khong chay duoc curl
inline bool chay_lenh(const std::string &lenh_json, std::string &phan_hoi) {
  static std::atomic<unsigned long long> dem{0};
  const char *d = getenv("TEMP");
  if (!d || !*d) d = getenv("TMPDIR");
  if (!d || !*d) d = getenv("TMP");
  if (!d || !*d) d = "/tmp";
  std::string tep =
      std::string(d) + "/hf_redis_" +
      std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) +
      "_" + std::to_string(dem++) + ".json";
  {
    std::ofstream out(tep, std::ios::binary);
    if (!out)
      return false;
    out << lenh_json;
  }
  const CauHinh &c = cau_hinh();
  std::string cmd = "curl -s -m 6 --connect-timeout 4 -X POST -H \"Authorization: Bearer " +
                    c.token + "\" -H \"Content-Type: application/json\" --data-binary \"@" +
                    tep + "\" \"" + c.url + "\"";
#ifdef _WIN32
  FILE *p = _popen(cmd.c_str(), "r");
#else
  FILE *p = popen(cmd.c_str(), "r");
#endif
  if (!p) {
    std::remove(tep.c_str());
    return false;
  }
  phan_hoi.clear();
  char buf[4096];
  size_t n;
  while ((n = fread(buf, 1, sizeof buf, p)) > 0)
    phan_hoi.append(buf, n);
#ifdef _WIN32
  _pclose(p);
#else
  pclose(p);
#endif
  std::remove(tep.c_str());
  return true;
}

// 1 = co (gia tri trong `gia_tri`), 0 = chua co, -1 = loi
inline int lay(const std::string &khoa, std::string &gia_tri) {
  if (!bat())
    return -1;
  std::string ph;
  if (!chay_lenh("[\"GET\"," + json_chuoi(khoa) + "]", ph))
    return -1;
  size_t a = ph.find("\"result\":");
  if (a == std::string::npos || ph.find("\"error\"") != std::string::npos)
    return -1;
  a += 9;
  while (a < ph.size() && ph[a] == ' ')
    a++;
  if (ph.compare(a, 4, "null") == 0)
    return 0;
  return doc_chuoi_json(ph, a, gia_tri) ? 1 : -1;
}

inline bool dat(const std::string &khoa, const std::string &gia_tri, int giay) {
  if (!bat() || gia_tri.size() > 900000)
    return false;
  std::string ph;
  if (!chay_lenh("[\"SET\"," + json_chuoi(khoa) + "," + json_chuoi(gia_tri) +
                     ",\"EX\"," + json_chuoi(std::to_string(giay)) + "]",
                 ph))
    return false;
  return ph.find("\"OK\"") != std::string::npos;
}

// Rut gon khoa qua dai bang bam FNV-1a 64 bit
inline std::string rut_gon(const std::string &khoa) {
  if (khoa.size() <= 180)
    return khoa;
  unsigned long long h = 1469598103934665603ULL;
  for (unsigned char c : khoa) {
    h ^= c;
    h *= 1099511628211ULL;
  }
  char b[32];
  snprintf(b, sizeof b, "%016llx", h);
  return khoa.substr(0, 40) + "~" + b;
}

} // namespace uc
