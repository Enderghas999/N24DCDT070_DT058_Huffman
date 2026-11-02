/* DT058 - Mã Huffman từ bảng tần suất nhỏ - giao diện.
   Mọi tính toán thuật toán nằm ở máy chủ C++ (huffman_core.hpp). Trang này chỉ
   hiển thị các sự kiện mà máy chủ ghi lại; mỗi sự kiện là một ảnh chụp đầy đủ
   trạng thái, nên lùi về bước trước luôn đưa cây, ds, ngăn xếp, mã giả và
   nhật ký về cùng một trạng thái. */
(() => {
  'use strict';
  const $ = (id) => document.getElementById(id);
  const esc = (s) => String(s).replace(/[&<>"]/g, (c) => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;' }[c]));
  const fmt = (n, d = 0) => Number(n).toLocaleString('vi-VN', { minimumFractionDigits: d, maximumFractionDigits: d });

  // ---------------- Theme ----------------
  const root = document.documentElement;
  try { const t = localStorage.getItem('dt058-theme'); if (t) root.dataset.theme = t; } catch (e) { /* bỏ qua */ }
  $('theme').onclick = () => {
    const dark = root.dataset.theme ? root.dataset.theme === 'dark' : matchMedia('(prefers-color-scheme: dark)').matches;
    root.dataset.theme = dark ? 'light' : 'dark';
    try { localStorage.setItem('dt058-theme', root.dataset.theme); } catch (e) { /* bỏ qua */ }
  };

  // ---------------- Tabs ----------------
  document.querySelectorAll('.tab').forEach((b) => {
    b.onclick = () => {
      document.querySelectorAll('.tab').forEach((x) => x.classList.toggle('active', x === b));
      document.querySelectorAll('.tabpane').forEach((p) => { p.hidden = p.id !== 'tab-' + b.dataset.tab; });
    };
  });

  // ---------------- Presets ----------------
  const PRESETS = [
    ['Ví dụ 1', 'A B C D', '5 9 12 13', 'ABCD'],
    ['Ví dụ 2', 'A B', '2 3', 'ABBA'],
    ['m = 1', 'A', '7', 'AAA'],
    ['1 1 1', 'A B C', '1 1 1', 'ABC'],
    ['1 1 8', 'A B C', '1 1 8', 'CCAB'],
    ['Tự thiết kế 1', 'A B C D', '1 4 2 5', 'ACDB'],
    ['Hòa 2 2 4 4 8', 'A B C D E', '2 2 4 4 8', 'EDCBA'],
    ['8 ký tự bằng nhau', 'A B C D E F G H', '5 5 5 5 5 5 5 5', 'HGFA'],
    ['Fibonacci', 'A B C D E F G H', '1 1 2 3 5 8 13 21', 'AH'],
  ];
  PRESETS.forEach((p) => {
    const b = document.createElement('button');
    b.className = 'btn'; b.textContent = p[0]; b.type = 'button';
    b.onclick = () => { $('kt').value = p[1]; $('f').value = p[2]; $('s').value = p[3]; };
    $('presets').appendChild(b);
  });

  // ---------------- Pseudocode (số dòng khớp trường "dong" của sự kiện) ----------------
  const CODE = [
    'tao_cay_huffman(f):',
    '  tạo lá i (trọng số f[i]) cho mỗi ký tự; ds ← [0, 1, …, m−1]',
    '  while |ds| > 1:',
    '    vt1 ← 0                                   // lay_hai_nho_nhat, lượt 1',
    '    for i ← 1..|ds|−1: if w(ds[i]) < w(ds[vt1]): vt1 ← i',
    '    vt2 ← (vt1 = 0) ? 1 : 0                   // lượt 2',
    '    for i ← 0..|ds|−1, i ∉ {vt1, vt2}: if w(ds[i]) < w(ds[vt2]): vt2 ← i',
    '    x ← ds[vt1];  y ← ds[vt2]',
    '    cha ← nút mới(w(x) + w(y), trái = x, phải = y); xóa x, y khỏi ds; thêm cha vào cuối ds',
    '  return ds[0]                                // gốc',
    'tao_ma(nut, tien_to):',
    '  if nut là lá: ma[nut] ← (tien_to rỗng ? "0" : tien_to); return',
    '  tao_ma(trái(nut), tien_to + "0")',
    '  tao_ma(phải(nut), tien_to + "1")',
    'tong_bit ← Σ f[i] × |ma[i]|',
  ];
  $('code').innerHTML = CODE.map((l) => '<li>' + esc(l) + '</li>').join('');
  const codeLines = [...$('code').children];

  const PHASE = { khoi_tao: 'Khởi tạo', quet: 'Tìm hai nút nhỏ nhất', chon: 'Chọn hai nút', ghep: 'Ghép nút', ma: 'Sinh mã (duyệt cây)', tong: 'Tính tổng bit' };
  const ROLE_TAG = { i: 'i', vt1: 'vt1', vt2: 'vt2', moi: 'mới' };

  // ---------------- State ----------------
  let R = null;          // kết quả máy chủ
  let cur = 0;           // bước hiện tại
  let timer = null;
  let quizAt = new Map(); // chỉ số sự kiện -> câu hỏi (chế độ luyện tập)
  let quizDone = new Set();
  let score = { ok: 0, all: 0 };
  let lastClient = null;

  const SPEEDS = [2000, 1200, 700, 350, 150];
  const speedLabel = ['0,5×', '0,75×', '1×', '2×', '4×'];
  $('speed').oninput = () => { $('speedV').textContent = speedLabel[$('speed').value - 1]; };
  $('speed').oninput();

  // ---------------- Run ----------------
  async function run() {
    stop();
    $('err').textContent = '';
    const p = new URLSearchParams({ kt: $('kt').value.trim(), f: $('f').value.trim(), s: $('s').value.trim(), che_do: $('mode').value });
    $('status').textContent = 'Đang gửi yêu cầu…';
    const t0 = performance.now();
    let res, data;
    try {
      res = await fetch('/api/chay?' + p.toString());
      data = await res.json();
    } catch (e) {
      $('err').textContent = 'Không kết nối được máy chủ.';
      $('status').textContent = 'Lỗi kết nối.';
      return;
    }
    const t1 = performance.now();
    if (!res.ok || data.loi) {
      $('err').textContent = data.loi || 'Lỗi không xác định.';
      $('status').textContent = 'Dữ liệu vào bị từ chối: ' + (data.loi || '');
      return;
    }
    R = data;
    const r0 = performance.now();
    buildQuiz();
    $('simEmpty').hidden = true; $('sim').hidden = false; $('resPanel').hidden = false;
    $('scrub').max = R.su_kien.length - 1;
    const nt = $('notice');
    nt.hidden = !R.bi_cat;
    nt.innerHTML = R.bi_cat ? '<b>Lưu ý:</b> số sự kiện mô phỏng vượt giới hạn nên chỉ hiển thị phần đầu; kết quả ở cột trái vẫn tính đầy đủ.' : '';
    renderResult();
    cur = R.che_do === 'tinh' ? R.su_kien.length - 1 : 0;
    show(cur);
    const r1 = performance.now();
    lastClient = { round: t1 - t0, render: r1 - r0 };
    renderEvalCurrent();
    loadHistory();
    if (R.che_do !== 'tinh') $('bPlay').focus();
  }
  $('run').onclick = run;
  ['kt', 'f', 's'].forEach((id) => $(id).addEventListener('keydown', (e) => { if (e.key === 'Enter') run(); }));

  // ---------------- Result panel ----------------
  function renderResult() {
    $('rBits').textContent = fmt(R.tong_bit);
    $('rRoot').textContent = R.cay[R.goc].w;
    $('rTable').tBodies[0].innerHTML = R.kt.map((c, i) =>
      `<tr><td class="c">${esc(c)}</td><td class="num">${R.f[i]}</td><td class="c">${R.ma[i]}</td><td class="num">${R.ma[i].length}</td><td class="num">${R.f[i] * R.ma[i].length}</td></tr>`).join('');
    $('rPrefix').innerHTML = 'Không mã nào là tiền tố của mã khác: ' + (R.khong_tien_to ? '<span class="ok">ĐÚNG</span>' : '<span class="bad">SAI</span>');
    $('rEnc').innerHTML = R.s
      ? `<p class="kv">“${esc(R.s)}” → <span class="mono bits">${R.bits}</span> (${R.bits.length} bit)<br>Giải mã lại: “${esc(R.giai_ma)}” ${R.giai_ma === R.s ? '<span class="ok">KHÔI PHỤC ĐÚNG</span>' : '<span class="bad">SAI</span>'}</p>`
      : '';
  }

  // ---------------- Simulation rendering ----------------
  function nodeName(id) {
    const n = R.cay[id];
    return n.la >= 0 ? `${R.kt[n.la]}(${n.w})` : `#${id}(${n.w})`;
  }

  function show(k) {
    if (!R) return;
    cur = Math.max(0, Math.min(k, R.su_kien.length - 1));
    const e = R.su_kien[cur];
    $('scrub').value = cur;
    $('pos').textContent = `Bước ${cur + 1}/${R.su_kien.length}`;
    $('phase').textContent = '· ' + (PHASE[e.pha] || '');
    $('explain').innerHTML = esc(e.loi);
    codeLines.forEach((li, i) => li.classList.toggle('on', i + 1 === e.dong));
    const on = codeLines[e.dong - 1];
    if (on && on.offsetParent) on.scrollIntoView({ block: 'nearest', inline: 'nearest' });
    drawTree(e);
    drawDs(e);
    $('counters').innerHTML =
      `<div>So sánh trọng số<b>${e.so_sanh}</b></div><div>Lần ghép<b>${e.so_lan_ghep}</b></div><div>Lời gọi tao_ma<b>${e.so_loi_goi}</b></div>`;
    $('stack').innerHTML = e.ngan_xep.map((f, i) => `<li><span class="lvl">${i}</span>${esc(f.ham)}(${esc(f.ts)})</li>`).join('');
    drawLog();
    $('status').textContent = cur === R.su_kien.length - 1
      ? `Hoàn tất: ${R.su_kien.length} bước, tổng bit = ${R.tong_bit}.`
      : `Đang mô phỏng bước ${cur + 1}/${R.su_kien.length} · ${PHASE[e.pha] || ''}`;
    $('bPrev').disabled = $('bFirst').disabled = cur === 0;
    $('bNext').disabled = $('bLast').disabled = cur === R.su_kien.length - 1;
    showQuizFor(cur);
  }

  function drawLog() {
    // Nhật ký = các sự kiện từ đầu đến bước hiện tại (lùi bước thì nhật ký lùi theo)
    const ol = $('log');
    const parts = [];
    for (let i = 0; i <= cur; i++) {
      parts.push(`<li class="${i === cur ? 'now' : ''}"><span class="n">${String(i + 1).padStart(3, ' ')}</span>${esc(R.su_kien[i].loi)}</li>`);
    }
    ol.innerHTML = parts.join('');
    const last = ol.lastElementChild;
    if (last) ol.scrollTop = last.offsetTop - ol.clientHeight + last.offsetHeight + 8;
  }

  function drawDs(e) {
    const roles = {}; // vị trí -> [vai trò]
    Object.entries(e.to_o).forEach(([r, i]) => { (roles[i] = roles[i] || []).push(r); });
    $('ds').innerHTML = e.ds.map((id, i) => {
      const rs = roles[i] || [];
      const cls = rs.map((r) => 'o-' + r).join(' ');
      return `<div class="cell ${cls}"><span class="ix">ds[${i}]</span><span class="nm">${esc(nodeName(id))}</span><span class="tags">${rs.map((r) => ROLE_TAG[r]).join(' · ')}</span></div>`;
    }).join('');
  }

  function drawTree(e) {
    const svg = $('tree');
    const roleOf = {};
    Object.entries(e.to_nut).forEach(([r, id]) => { if (!(id in roleOf) || r === 'dang_xet' || r === 'moi' || r === 'la_ma') roleOf[id] = r; });
    // Các gốc hiện có: trong pha dựng cây là ds; sau đó là gốc của cây
    const roots = e.ds;
    const pos = {};
    let X = 0, maxD = 0;
    const lay = (id, d) => {
      const n = R.cay[id];
      maxD = Math.max(maxD, d);
      if (n.trai < 0) { pos[id] = { x: X++, d }; return; }
      lay(n.trai, d + 1); lay(n.phai, d + 1);
      pos[id] = { x: (pos[n.trai].x + pos[n.phai].x) / 2, d };
    };
    roots.forEach((r, i) => { if (i) X += 0.5; lay(r, 0); });
    const DX = 58, DY = 66, RAD = 18;
    const W = Math.max(X * DX + 20, 300), H = (maxD + 1) * DY + 56;
    const px = (id) => pos[id].x * DX + DX / 2 + 10, py = (id) => pos[id].d * DY + 30;
    let g = '';
    Object.keys(pos).forEach((k) => {
      const n = R.cay[k];
      if (n.trai < 0) return;
      [[n.trai, '0'], [n.phai, '1']].forEach(([c, t]) => {
        g += `<line class="edge" x1="${px(k)}" y1="${py(k)}" x2="${px(c)}" y2="${py(c)}"/>`;
        const mx = (px(k) + px(c)) / 2 + (t === '0' ? -8 : 8), my = (py(k) + py(c)) / 2 - 2;
        g += `<text class="elab" x="${mx}" y="${my}" text-anchor="middle">${t}</text>`;
      });
    });
    Object.keys(pos).forEach((k) => {
      const n = R.cay[k], leaf = n.trai < 0, role = roleOf[k];
      g += `<circle class="n ${leaf ? 'leaf' : ''} ${role ? 'r-' + role : ''}" cx="${px(k)}" cy="${py(k)}" r="${RAD}"><title>${esc(nodeName(+k))}</title></circle>`;
      g += `<text x="${px(k)}" y="${py(k) + 5}" text-anchor="middle" font-size="13" font-weight="700">${n.w}</text>`;
      if (leaf) {
        g += `<text x="${px(k)}" y="${py(k) + RAD + 15}" text-anchor="middle" font-size="14" font-weight="800">${esc(R.kt[n.la])}</text>`;
        const code = e.ma[n.la];
        if (code) g += `<text class="code-t" x="${px(k)}" y="${py(k) + RAD + 29}" text-anchor="middle">${code}</text>`;
      } else {
        g += `<text x="${px(k)}" y="${py(k) - RAD - 4}" text-anchor="middle" font-size="10" class="muted" style="fill:var(--muted)">#${k}</text>`;
      }
    });
    svg.setAttribute('viewBox', `0 0 ${W} ${H}`);
    svg.setAttribute('width', W); svg.setAttribute('height', H);
    svg.innerHTML = g;
  }

  // ---------------- Controls ----------------
  function stop() {
    if (timer) clearTimeout(timer);
    timer = null;
    $('bPlay').textContent = '▶ Tự chạy';
  }
  function tick() {
    if (!R || cur >= R.su_kien.length - 1) { stop(); return; }
    if (quizBlocks(cur + 1)) { show(cur + 1); stop(); return; }
    show(cur + 1);
    timer = setTimeout(tick, SPEEDS[$('speed').value - 1]);
  }
  $('bPlay').onclick = () => {
    if (!R) return;
    if (timer) { stop(); return; }
    if (cur >= R.su_kien.length - 1) show(0);
    if (quizBlocks(cur)) return;
    $('bPlay').textContent = '⏸ Tạm dừng';
    timer = setTimeout(tick, SPEEDS[$('speed').value - 1]);
  };
  $('bNext').onclick = () => { stop(); if (!quizBlocks(cur)) show(cur + 1); };
  $('bPrev').onclick = () => { stop(); show(cur - 1); };
  $('bFirst').onclick = () => { stop(); show(0); };
  $('bLast').onclick = () => {
    stop();
    if (R && R.che_do === 'luyen') { // không cho nhảy qua câu hỏi chưa trả lời
      for (let i = cur; i < R.su_kien.length; i++) if (quizAt.has(i) && !quizDone.has(i)) { show(i); return; }
    }
    show(R.su_kien.length - 1);
  };
  $('scrub').oninput = () => {
    stop();
    let k = +$('scrub').value;
    if (R.che_do === 'luyen') for (let i = cur; i < k; i++) if (quizAt.has(i) && !quizDone.has(i)) { k = i; break; }
    show(k);
  };
  document.addEventListener('keydown', (e) => {
    if (!R || /INPUT|SELECT|TEXTAREA/.test(document.activeElement.tagName)) return;
    if (e.key === 'ArrowRight') $('bNext').click();
    else if (e.key === 'ArrowLeft') $('bPrev').click();
  });

  // ---------------- Practice mode ----------------
  // Câu hỏi được gắn vào một sự kiện; người học phải trả lời trước khi xem bước kế tiếp.
  function buildQuiz() {
    quizAt = new Map(); quizDone = new Set(); score = { ok: 0, all: 0 };
    if (R.che_do !== 'luyen') return;
    const ev = R.su_kien;
    const w = (id) => R.cay[id].w;
    let firstMerge = true, tieAsked = false, codeAsked = false;
    ev.forEach((e, i) => {
      // Trước khi chọn: dự đoán hai nút sẽ được ghép. Đặt câu hỏi tại sự kiện "ds còn k nút" (dòng 3)
      if (e.pha === 'quet' && e.dong === 3) {
        const j = ev.findIndex((x, t) => t > i && x.pha === 'chon');
        if (j < 0) return;
        const c = ev[j], x = c.ds[c.to_o.vt1], y = c.ds[c.to_o.vt2];
        const pairs = [];
        for (let a = 0; a < e.ds.length; a++) for (let b = a + 1; b < e.ds.length; b++) pairs.push([e.ds[a], e.ds[b]]);
        const key = (p) => [...p].sort((u, v) => u - v).join('-');
        const right = key([x, y]);
        const others = pairs.filter((p) => key(p) !== right).sort((p, q) => (w(p[0]) + w(p[1])) - (w(q[0]) + w(q[1]))).slice(0, 3);
        const opts = [[x, y], ...others].map((p) => ({ t: nodeName(p[0]) + ' và ' + nodeName(p[1]), ok: key(p) === right }));
        opts.sort((p, q) => p.t.localeCompare(q.t));
        quizAt.set(i, {
          q: `ds hiện có ${e.ds.length} nút. Hai nút nào sẽ được ghép tiếp theo?`,
          opts, hint: 'Thuật toán tham lam luôn lấy hai nút có trọng số nhỏ nhất; khi bằng nhau, nút đứng trước trong ds được ưu tiên.',
          why: c.loi,
        });
        if (firstMerge) {
          const g = ev.findIndex((x2, t) => t > j && x2.pha === 'ghep');
          quizAt.set(j, {
            q: `Nút cha tạo từ ${nodeName(x)} và ${nodeName(y)} có trọng số bao nhiêu?`, num: w(x) + w(y),
            hint: 'Trọng số nút cha bằng tổng trọng số hai con.', why: ev[g].loi,
          });
          firstMerge = false;
        }
      }
      if (!tieAsked && e.pha === 'quet' && /bằng nhau/.test(e.loi)) {
        tieAsked = true;
        if (quizAt.has(i)) return;
        quizAt.set(i, {
          q: 'Hai nút có trọng số bằng nhau. Vì sao chương trình giữ nút đứng trước?',
          opts: [
            { t: 'Vì phép so sánh dùng dấu < (nhỏ hơn hẳn) nên chỉ cập nhật khi gặp nút nhẹ hơn; đây là quy ước cố định, không làm thay đổi tổng bit', ok: true },
            { t: 'Vì nút đứng trước luôn là lá nên phải ghép trước', ok: false },
            { t: 'Vì nếu chọn nút đứng sau thì mã sẽ có tiền tố trùng nhau', ok: false },
          ],
          hint: 'Xem dòng mã giả đang tô sáng: điều kiện cập nhật là “<”, không phải “≤”.',
          why: 'Với “<”, nút đến sau có trọng số bằng sẽ không thay thế nút đã chọn. Đổi quy ước khi hòa có thể đổi mã cụ thể nhưng không đổi tổng bit tối thiểu.',
        });
      }
      if (!codeAsked && e.pha === 'ma' && e.dong === 12) {
        codeAsked = true;
        const leaf = e.to_nut.la_ma, ch = R.kt[R.cay[leaf].la];
        // hỏi trước sự kiện gán mã (tại sự kiện ngay trước đó)
        quizAt.set(i - 1, {
          q: `Khi tao_ma tới lá ${ch}, mã của ${ch} sẽ là gì? (nhìn ngăn xếp lời gọi)`, text: e.ma[R.cay[leaf].la],
          hint: 'Mã là tiền tố đang mang theo: mỗi lần sang trái thêm 0, sang phải thêm 1. Nếu cây chỉ có một lá thì dùng “0”.',
          why: e.loi,
        });
      }
      if (e.pha === 'tong') {
        quizAt.set(i - 1, {
          q: 'Tổng số bit của bảng mã bằng bao nhiêu?', num: R.tong_bit,
          hint: R.m > 1 ? 'Tổng bit = Σ f × độ dài mã, cũng bằng tổng trọng số các nút cha.' : 'Một ký tự có mã “0” dài 1 bit.',
          why: e.loi,
        });
      }
    });
    if (R.m === 1) {
      quizAt.set(0, {
        q: 'Bảng chỉ có một ký tự. Vì sao chương trình gán mã “0” thay vì chuỗi rỗng?',
        opts: [
          { t: 'Vì mã rỗng dài 0 bit nên không thể biểu diễn hay đếm số lần ký tự xuất hiện trong chuỗi bit', ok: true },
          { t: 'Vì cây có hai lá', ok: false },
          { t: 'Vì tần suất phải lớn hơn 1', ok: false },
        ],
        hint: 'Thử mã hóa chuỗi “AAA” bằng mã rỗng.', why: 'Với mã rỗng, “AAA” mã hóa thành chuỗi rỗng và không giải mã lại được. Dùng “0” thì “AAA” → “000”.',
      });
    }
  }

  function quizBlocks(k) { return R && R.che_do === 'luyen' && quizAt.has(k) && !quizDone.has(k); }

  function showQuizFor(k) {
    const box = $('quiz');
    const q = R.che_do === 'luyen' ? quizAt.get(k) : null;
    if (!q) { box.hidden = true; return; }
    box.hidden = false;
    const done = quizDone.has(k);
    let body = `<span class="score">Điểm: ${score.ok}/${score.all}</span><h4>Dự đoán trước khi xem bước tiếp</h4><p>${esc(q.q)}</p>`;
    if (q.opts) body += '<div class="opts">' + q.opts.map((o, i) => `<button class="btn" data-i="${i}" ${done ? 'disabled' : ''}>${esc(o.t)}</button>`).join('') + '</div>';
    else body += `<div class="row"><input id="qIn" type="text" ${done ? 'disabled' : ''} placeholder="Nhập câu trả lời"><button class="btn primary" id="qOk" ${done ? 'disabled' : ''}>Kiểm tra</button></div>`;
    body += `<button class="link" id="qHint">Gợi ý</button><div class="fb" id="qFb">${done ? '<span class="ok">Đã trả lời.</span> ' + esc(q.why) : ''}</div>`;
    box.innerHTML = body;
    $('qHint').onclick = () => { if (!quizDone.has(k)) $('qFb').innerHTML = '<b>Gợi ý:</b> ' + esc(q.hint); };
    const finish = (ok, shown) => {
      quizDone.add(k); score.all++; if (ok) score.ok++;
      $('qFb').innerHTML = (ok ? '<span class="ok">Đúng.</span> ' : `<span class="bad">Chưa đúng.</span> Đáp án: <b>${esc(shown)}</b>. `) + esc(q.why) + ' <button class="btn" id="qGo">Xem bước tiếp ▶</button>';
      box.querySelectorAll('button[data-i], #qOk, #qIn').forEach((b) => { b.disabled = true; });
      box.querySelector('.score').textContent = `Điểm: ${score.ok}/${score.all}`;
      $('qGo').onclick = () => show(k + 1);
      $('qGo').focus();
    };
    if (done) return;
    if (q.opts) {
      box.querySelectorAll('button[data-i]').forEach((b) => {
        b.onclick = () => {
          const o = q.opts[+b.dataset.i];
          b.classList.add(o.ok ? 'chosen-ok' : 'chosen-bad');
          box.querySelectorAll('button[data-i]').forEach((x) => { if (q.opts[+x.dataset.i].ok) x.classList.add('chosen-ok'); });
          finish(o.ok, q.opts.find((x) => x.ok).t);
        };
      });
    } else {
      const check = () => {
        const v = $('qIn').value.trim();
        if (!v) return;
        const ok = q.num !== undefined ? Number(v) === q.num : v === q.text;
        finish(ok, q.num !== undefined ? String(q.num) : q.text);
      };
      $('qOk').onclick = check;
      $('qIn').onkeydown = (e) => { if (e.key === 'Enter') check(); };
    }
  }

  // ---------------- Evaluation: current run ----------------
  const us = (ns) => ns < 1000 ? `${fmt(ns)} ns` : ns < 1e6 ? `${fmt(ns / 1000, 2)} µs` : `${fmt(ns / 1e6, 2)} ms`;
  function renderEvalCurrent() {
    if (!R) return;
    const d = R.dem, t = R.thoi_gian, c = lastClient;
    const transfer = Math.max(0, c.round - R.xu_ly_server_ms);
    const M = (l, v, desc) => `<div class="metric"><div class="l">${l}</div><div class="v">${v}</div><div class="d">${desc}</div></div>`;
    $('evEmpty').hidden = true;
    $('evCur').hidden = false;
    $('evCur').innerHTML =
      `<p class="muted">Bảng: ${esc(R.kt.join(' '))} · f = ${R.f.join(' ')} · m = ${R.m}</p>` +
      '<h3 class="small">Bộ đếm (định nghĩa cố định, đếm trong mã C++)</h3><div class="metrics">' +
      M('Số phép so sánh trọng số', d.so_sanh, `Tăng 1 mỗi lần so sánh w(ds[i]) với nút đang giữ trong lay_hai_nho_nhat. Công thức (m − 1)² = ${d.cong_thuc_so_sanh}.`) +
      M('Số lần ghép', d.so_lan_ghep, 'Mỗi vòng while tạo một nút cha; luôn bằng m − 1.') +
      M('Số nút của cây', d.so_nut, 'm lá + (m − 1) nút cha = 2m − 1.') +
      M('Số lời gọi tao_ma', d.so_loi_goi, 'Một lời gọi cho mỗi nút của cây (2m − 1).') +
      M('Khối lượng đầu ra', d.khoi_luong_dau_ra + ' bit', 'Σ độ dài các mã trong bảng mã (số ký tự 0/1 phải ghi ra).') +
      M('Tổng số bit', R.tong_bit, 'Σ f[i] × |ma[i]|, bằng tổng trọng số các nút cha.') +
      '</div><h3 class="small">Thời gian (tách theo nhiệm vụ)</h3><div class="metrics">' +
      M('1. Tính tổng bit', us(t.tinh_tong_bit.trung_vi_ns), `Chỉ dựng cây và cộng trọng số nút cha. Trung vị ${t.tinh_tong_bit.lap} mẫu, mỗi mẫu ${fmt(t.tinh_tong_bit.lo)} lần chạy (min ${us(t.tinh_tong_bit.nho_nhat_ns)}, max ${us(t.tinh_tong_bit.lon_nhat_ns)}).`) +
      M('2. Sinh đầy đủ bảng mã', us(t.sinh_bang_ma.trung_vi_ns), `Dựng cây + duyệt cây tạo mọi mã + tính tổng bit; không ghi vết. Trung vị ${t.sinh_bang_ma.lap} mẫu (min ${us(t.sinh_bang_ma.nho_nhat_ns)}, max ${us(t.sinh_bang_ma.lon_nhat_ns)}).`) +
      M('3. Ghi vết mô phỏng', us(t.ghi_vet_ns), `Chạy một lần có ghi ${R.su_kien.length} sự kiện (chuỗi JSON). Đây là chi phí phục vụ hiển thị, không phải của thuật toán.`) +
      M('4. Truyền dữ liệu', `${fmt(transfer, 1)} ms`, `Thời gian vòng yêu cầu (${fmt(c.round, 1)} ms) trừ thời gian xử lý ở máy chủ (${fmt(R.xu_ly_server_ms, 1)} ms).`) +
      M('5. Hiển thị', `${fmt(c.render, 1)} ms`, 'Thời gian trình duyệt dựng bảng mã, cây và nhật ký lần đầu (performance.now).') +
      '</div><p class="muted small">Thời gian chờ người dùng bấm nút và hoạt ảnh tự chạy không được tính vào bất kỳ mục nào ở trên.</p>';
  }

  // ---------------- Evaluation: experiment ----------------
  let abortCtl = null;
  $('bRun').onclick = async () => {
    $('bErr').textContent = '';
    const p = new URLSearchParams({ m: $('bM').value.replace(/\s+/g, ''), lap: $('bLap').value, hat: $('bSeed').value });
    abortCtl = new AbortController();
    $('bRun').disabled = true; $('bStop').disabled = false;
    $('bOut').innerHTML = '<p class="muted">Đang đo… (m lớn có thể mất vài giây)</p>';
    try {
      const [res, env] = await Promise.all([
        fetch('/api/danh_gia?' + p, { signal: abortCtl.signal }),
        fetch('/api/moi_truong', { signal: abortCtl.signal }).then((r) => r.json()),
      ]);
      const d = await res.json();
      if (!res.ok || d.loi) { $('bErr').textContent = d.loi || 'Lỗi.'; $('bOut').innerHTML = ''; return; }
      renderBench(d, env);
    } catch (e) {
      $('bOut').innerHTML = e.name === 'AbortError' ? '<p class="muted">Đã dừng (trang ngừng chờ kết quả).</p>' : '';
      if (e.name !== 'AbortError') $('bErr').textContent = 'Không kết nối được máy chủ.';
    } finally {
      $('bRun').disabled = false; $('bStop').disabled = true; abortCtl = null;
    }
  };
  $('bStop').onclick = () => { if (abortCtl) abortCtl.abort(); };

  function slope(xs, ys) { // hệ số góc hồi quy tuyến tính trên log-log
    const lx = xs.map(Math.log), ly = ys.map(Math.log), n = xs.length;
    const mx = lx.reduce((a, b) => a + b, 0) / n, my = ly.reduce((a, b) => a + b, 0) / n;
    let num = 0, den = 0;
    for (let i = 0; i < n; i++) { num += (lx[i] - mx) * (ly[i] - my); den += (lx[i] - mx) ** 2; }
    return den ? num / den : NaN;
  }

  function renderBench(d, env) {
    const k = d.ket_qua;
    let html = `<p class="muted small">Môi trường đo: ${esc(env.he_dieu_hanh)} · ${esc(env.trinh_bien_dich)} · tối ưu hóa ${esc(env.toi_uu)} · ${env.so_luong_cpu} luồng CPU · đồng hồ ${esc(env.dong_ho)} · ${d.lap} lần lặp, lấy trung vị · hạt giống ${d.hat}.</p>`;
    html += '<div class="tbl-wrap"><table class="tbl"><thead><tr><th class="num">m</th><th class="num">Tổng bit</th><th class="num">So sánh đo được</th><th class="num">(m − 1)²</th><th class="num">Lời gọi tao_ma</th><th class="num">Mã dài nhất</th><th class="num">Khối lượng đầu ra (bit)</th><th class="num">Tính tổng bit</th><th class="num">Sinh bảng mã</th></tr></thead><tbody>';
    html += k.map((r) => `<tr><td class="num">${fmt(r.m)}</td><td class="num">${fmt(r.tong_bit)}</td><td class="num">${fmt(r.so_sanh)}</td><td class="num">${fmt(r.cong_thuc)}</td><td class="num">${fmt(r.so_loi_goi)}</td><td class="num">${r.ma_dai_nhat}</td><td class="num">${fmt(r.khoi_luong)}</td><td class="num">${us(r.tinh_tong_bit.trung_vi_ns)}</td><td class="num">${us(r.sinh_bang_ma.trung_vi_ns)}</td></tr>`).join('');
    html += '</tbody></table></div><div class="charts"><div class="chart" id="c1"></div><div class="chart" id="c2"></div></div>';
    // Nhận xét: chỉ dùng số đo được
    const big = k.filter((r) => r.m >= 32);
    const allMatch = k.every((r) => r.so_sanh === r.cong_thuc);
    let note = '<div class="note"><b>Nhận xét (tính từ số liệu ở bảng trên)</b><ul>';
    note += `<li>Số phép so sánh đo được ${allMatch ? 'trùng khớp' : '<b>KHÔNG</b> trùng khớp'} với (m − 1)² ở ${allMatch ? 'mọi' : 'một số'} giá trị m: mỗi vòng với k nút cần (k − 1) + (k − 2) = 2k − 3 phép, cộng từ k = 2 đến m được (m − 1)².</li>`;
    if (big.length >= 2) {
      const s1 = slope(big.map((r) => r.m), big.map((r) => r.tinh_tong_bit.trung_vi_ns));
      const s2 = slope(big.map((r) => r.m), big.map((r) => r.sinh_bang_ma.trung_vi_ns));
      note += `<li>Với m ≥ 32, hệ số góc ước lượng trên thang log–log là ${fmt(s1, 2)} (tính tổng bit) và ${fmt(s2, 2)} (sinh bảng mã); khi m gấp đôi, thời gian tăng khoảng ${fmt(2 ** s1, 1)} và ${fmt(2 ** s2, 1)} lần. Nếu đúng O(m²) thì hệ số góc là 2 (tăng 4 lần). Giá trị đo được ${Math.abs(s1 - 2) < 0.3 && Math.abs(s2 - 2) < 0.3 ? 'gần với 2' : 'lệch khỏi 2 đáng kể; nên chạy lại với nhiều lần lặp hơn'}; sai lệch có thể do bộ nhớ đệm CPU và nhiễu của máy đo.</li>`;
    } else note += '<li>Cần ít nhất hai giá trị m ≥ 32 để ước lượng tốc độ tăng của thời gian.</li>';
    const small = k.filter((r) => r.m <= 8);
    if (small.length) note += `<li>Trong miền của phiếu (m ≤ 8), sinh bảng mã mất ${us(Math.min(...small.map((r) => r.sinh_bang_ma.trung_vi_ns)))} – ${us(Math.max(...small.map((r) => r.sinh_bang_ma.trung_vi_ns)))}; ở mức này thời gian truyền và hiển thị trên trình duyệt (tab “Lần chạy hiện tại”) lớn hơn thời gian thuật toán nhiều lần.</li>`;
    const last = k[k.length - 1];
    const t1 = last.tinh_tong_bit, t2 = last.sinh_bang_ma;
    const overlap = t1.nho_nhat_ns <= t2.lon_nhat_ns && t2.nho_nhat_ns <= t1.lon_nhat_ns;
    note += `<li>Ở m = ${fmt(last.m)}, sinh bảng mã (thêm bước duyệt cây và ghi ${fmt(last.khoi_luong)} bit mã) mất ${us(t2.trung_vi_ns)} so với ${us(t1.trung_vi_ns)} của riêng phần tính tổng bit, tỉ lệ ${fmt(t2.trung_vi_ns / t1.trung_vi_ns, 2)}. ` +
      (overlap ? 'Khoảng [min, max] của hai phép đo chồng lên nhau nên chênh lệch này nằm trong sai số đo; phần duyệt cây O(m · L) nhỏ so với phần quét O(m²).' : 'Khoảng [min, max] của hai phép đo không chồng nhau.') + '</li></ul></div>';
    $('bOut').innerHTML = html + note;
    lineChart($('c1'), 'Thời gian theo m (trung vị, thang log–log)', k.map((r) => r.m), [
      { name: 'Tính tổng bit', color: 'var(--s1)', ys: k.map((r) => r.tinh_tong_bit.trung_vi_ns / 1000) },
      { name: 'Sinh bảng mã', color: 'var(--s2)', ys: k.map((r) => r.sinh_bang_ma.trung_vi_ns / 1000) },
    ], 'µs');
    lineChart($('c2'), 'Số phép so sánh: đo được và (m − 1)² (log–log; hai đường trùng nhau khi khớp)', k.map((r) => r.m), [
      { name: 'Đo được', color: 'var(--s1)', ys: k.map((r) => Math.max(r.so_sanh, 0.5)), raw: k.map((r) => r.so_sanh) },
      { name: '(m − 1)²', color: 'var(--s2)', dash: true, ys: k.map((r) => Math.max(r.cong_thuc, 0.5)), raw: k.map((r) => r.cong_thuc) },
    ], 'phép');
  }

  function lineChart(el, title, xs, series, unit) {
    const W = 460, H = 260, L = 56, Rr = 12, T = 12, B = 34;
    const allY = series.flatMap((s) => s.ys).filter((v) => v > 0);
    const lx0 = Math.log10(Math.min(...xs)), lx1 = Math.log10(Math.max(...xs));
    let ly0 = Math.floor(Math.log10(Math.min(...allY))), ly1 = Math.ceil(Math.log10(Math.max(...allY)));
    if (ly0 === ly1) ly1++;
    const X = (v) => L + (lx1 === lx0 ? 0.5 : (Math.log10(v) - lx0) / (lx1 - lx0)) * (W - L - Rr);
    const Y = (v) => T + (1 - (Math.log10(v) - ly0) / (ly1 - ly0)) * (H - T - B);
    let s = '';
    for (let e = ly0; e <= ly1; e++) {
      const y = Y(10 ** e);
      s += `<line class="gl" x1="${L}" x2="${W - Rr}" y1="${y}" y2="${y}"/><text x="${L - 6}" y="${y + 4}" text-anchor="end">${fmt(10 ** e, e < 0 ? -e : 0)}</text>`;
    }
    xs.forEach((x) => { s += `<text x="${X(x)}" y="${H - B + 16}" text-anchor="middle">${fmt(x)}</text>`; });
    s += `<text x="${(L + W - Rr) / 2}" y="${H - 4}" text-anchor="middle">m (số ký tự)</text>`;
    s += `<text x="12" y="${(T + H - B) / 2}" text-anchor="middle" transform="rotate(-90 12 ${(T + H - B) / 2})">${unit}</text>`;
    series.forEach((sr) => {
      const pts = xs.map((x, i) => `${X(x)},${Y(sr.ys[i])}`).join(' ');
      s += `<polyline fill="none" stroke="${sr.color}" stroke-width="2" ${sr.dash ? 'stroke-dasharray="6 4"' : ''} points="${pts}"/>`;
      xs.forEach((x, i) => { s += `<circle cx="${X(x)}" cy="${Y(sr.ys[i])}" r="4" fill="${sr.color}" stroke="var(--panel)" stroke-width="2"/>`; });
    });
    // vùng hover theo cột x
    xs.forEach((x, i) => {
      const x0 = i ? (X(xs[i - 1]) + X(x)) / 2 : L, x1 = i < xs.length - 1 ? (X(x) + X(xs[i + 1])) / 2 : W - Rr;
      s += `<rect class="hov" data-i="${i}" x="${x0}" y="${T}" width="${Math.max(1, x1 - x0)}" height="${H - T - B}" fill="transparent"/>`;
    });
    el.innerHTML = `<h4>${esc(title)}</h4><div class="lg">${series.map((sr) => `<span><i style="background:${sr.color}"></i>${esc(sr.name)}</span>`).join('')}</div><svg viewBox="0 0 ${W} ${H}" role="img" aria-label="${esc(title)}">${s}<line class="ax cross" x1="0" x2="0" y1="${T}" y2="${H - B}" style="display:none;stroke:var(--muted)"/></svg>`;
    const svg = el.querySelector('svg'), cross = el.querySelector('.cross'), tip = $('tip');
    el.querySelectorAll('.hov').forEach((r) => {
      r.addEventListener('mousemove', (ev) => {
        const i = +r.dataset.i;
        cross.style.display = ''; cross.setAttribute('x1', X(xs[i])); cross.setAttribute('x2', X(xs[i]));
        tip.hidden = false;
        tip.innerHTML = `<b>m = ${fmt(xs[i])}</b><br>` + series.map((sr) => `${esc(sr.name)}: ${sr.raw ? fmt(sr.raw[i]) : fmt(sr.ys[i], sr.ys[i] < 10 ? 3 : 1)} ${unit}`).join('<br>');
        tip.style.left = Math.min(ev.clientX + 14, innerWidth - 270) + 'px'; tip.style.top = (ev.clientY + 14) + 'px';
      });
      r.addEventListener('mouseleave', () => { cross.style.display = 'none'; tip.hidden = true; });
    });
    svg.addEventListener('mouseleave', () => { tip.hidden = true; });
  }

  // ---------------- Tests ----------------
  $('tRun').onclick = async () => {
    $('tOut').innerHTML = '<p class="muted">Đang chạy…</p>';
    $('tRun').disabled = true;
    try {
      const d = await (await fetch('/api/kiem_tra?' + new URLSearchParams({ n: $('tN').value, hat: $('tSeed').value }))).json();
      const all = d.ca_dat === d.ca_tong;
      let h = `<div class="sum"><span class="pill ${all ? 'ok' : 'bad'}">${d.ca_dat}/${d.ca_tong} ca đạt</span><span class="pill ${d.pk_dat === d.pk_tong ? 'ok' : 'bad'}">${d.pk_dat}/${d.pk_tong} phép kiểm đạt</span><span class="muted small">${fmt(d.n)} bảng ngẫu nhiên, hạt giống ${d.hat}, ${fmt(d.thoi_gian_ms, 1)} ms</span></div>`;
      h += d.ca.map((c) => `<details class="case" ${c.dat ? '' : 'open'}><summary><span class="${c.dat ? 'ok' : 'bad'}">${c.dat ? 'ĐẠT' : 'SAI'}</span><span class="id">${esc(c.ma)}</span><span>${esc(c.mo_ta)}</span><span class="grp">${esc(c.nhom)}</span></summary><ul>` +
        c.kiem.map((k) => `<li><span class="${k.dat ? 'ok' : 'bad'}">${k.dat ? '✓' : '✗'}</span> ${esc(k.mo_ta)} ${k.chi_tiet ? `<span class="det">— ${esc(k.chi_tiet)}</span>` : ''}</li>`).join('') + '</ul></details>').join('');
      $('tOut').innerHTML = h;
    } catch (e) {
      $('tOut').innerHTML = '<p class="bad">Không kết nối được máy chủ.</p>';
    } finally { $('tRun').disabled = false; }
  };

  // ---------------- History ----------------
  const MODE = { tinh: 'Tính', hoc: 'Học', luyen: 'Luyện' };
  async function loadHistory() {
    try {
      const [h, env] = await Promise.all([fetch('/api/lich_su?gioi_han=15').then((r) => r.json()), fetch('/api/moi_truong').then((r) => r.json())]);
      $('histStore').textContent = env.luu_lich_su.startsWith('PostgreSQL') ? 'PostgreSQL' : 'tệp';
      $('histStore').title = 'Lịch sử dùng chung cho mọi người truy cập; lưu ở ' + env.luu_lich_su;
      const tb = $('hist').tBodies[0];
      if (!h.length) { tb.innerHTML = '<tr><td colspan="4" class="muted">Chưa có lần chạy nào.</td></tr>'; return; }
      tb.innerHTML = h.map((r, i) => {
        const t = new Date(r.thoi_diem);
        const p2 = (v) => String(v).padStart(2, '0');
        const ts = isNaN(t) ? r.thoi_diem : `${p2(t.getDate())}/${p2(t.getMonth() + 1)} ${p2(t.getHours())}:${p2(t.getMinutes())}`;
        return `<tr><td>${esc(ts)}<br><span class="muted">${MODE[r.che_do] || ''}</span></td><td class="mono">${esc(r.kt)}<br><span class="muted">${esc(r.f)}</span></td><td class="num">${r.tong_bit}</td>` +
          `<td class="act"><button class="link" data-i="${i}" data-a="open">Mở lại</button><button class="link" data-i="${i}" data-a="run">Chạy lại</button></td></tr>`;
      }).join('');
      tb.querySelectorAll('button').forEach((b) => {
        b.onclick = () => {
          const r = h[+b.dataset.i];
          $('kt').value = r.kt; $('f').value = r.f; $('s').value = r.s || ''; $('mode').value = r.che_do || 'hoc';
          if (b.dataset.a === 'run') run(); else $('kt').focus();
        };
      });
    } catch (e) {
      $('hist').tBodies[0].innerHTML = '<tr><td colspan="4" class="muted">Không tải được lịch sử.</td></tr>';
    }
  }
  loadHistory();
})();
