#pragma once
#include <Arduino.h>

static const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
  <meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
  <meta charset="utf-8">
  <title>Receiver Dashboard</title>
  <style>
    :root{
      --bg:#0b0d10;
      --panel:#12161c;
      --panel2:#0f1318;
      --text:#e9eef6;
      --muted:#a7b2c2;
      --line:#263041;
      --good:#40d17a;
      --warn:#f6c453;
      --bad:#ff5a6a;
      --chip:#1b2230;
      --shadow: 0 10px 30px rgba(0,0,0,.35);
      --radius:16px;
      --n1:#4f8cff;
      --n2:#40d17a;
    }
    @media (prefers-color-scheme: light){
      :root{
        --bg:#f6f8fb;
        --panel:#ffffff;
        --panel2:#fbfcfe;
        --text:#0f172a;
        --muted:#475569;
        --line:#e2e8f0;
        --chip:#eef2f7;
        --shadow: 0 12px 26px rgba(2,6,23,.10);
      }
    }

    *{box-sizing:border-box}
    body{
      margin:0;
      font-family: ui-sans-serif, system-ui, -apple-system, Segoe UI, Roboto, Helvetica, Arial;
      background: radial-gradient(1200px 600px at 20% -10%, rgba(79,140,255,.18), transparent 60%),
                  radial-gradient(900px 500px at 90% 0%, rgba(64,209,122,.14), transparent 55%),
                  var(--bg);
      color:var(--text);
    }

    .wrap{max-width:1180px;margin:0 auto;padding:18px 14px 28px}
    header{
      display:flex;gap:14px;align-items:center;justify-content:space-between;flex-wrap:wrap;
      padding:10px 4px 14px;
    }
    h1{margin:0;font-size:20px;letter-spacing:.2px}
    .sub{color:var(--muted);font-size:13px;line-height:1.35}
    code{background:var(--chip);padding:2px 8px;border-radius:10px;border:1px solid var(--line)}

    .status{
      display:flex;align-items:center;gap:10px;
      padding:10px 12px;border:1px solid var(--line);border-radius:999px;background:rgba(255,255,255,.02);
      box-shadow: var(--shadow);
      min-width:260px; justify-content:space-between;
    }
    .pill{
      display:inline-flex;align-items:center;gap:8px;
      padding:6px 10px;border-radius:999px;font-weight:800;font-size:12px;
      background:var(--chip);border:1px solid var(--line);
    }
    .dot{width:10px;height:10px;border-radius:50%}
    .pill.good .dot{background:var(--good)}
    .pill.bad  .dot{background:var(--bad)}
    .pill.warn .dot{background:var(--warn)}
    .meta{font-size:12px;color:var(--muted)}
    .meta div{white-space:nowrap}

    .panel{
      background:linear-gradient(180deg, rgba(255,255,255,.04), rgba(255,255,255,.00));
      background-color:var(--panel);
      border:1px solid var(--line);
      border-radius:var(--radius);
      box-shadow: var(--shadow);
      overflow:hidden;
    }
    .panel .hd{
      display:flex;align-items:center;justify-content:space-between;gap:10px;
      padding:14px 14px 10px;
      border-bottom:1px solid var(--line);
      background:linear-gradient(180deg, rgba(255,255,255,.04), transparent);
    }
    .panel .hd .t{font-size:14px;font-weight:900;letter-spacing:.2px}
    .panel .bd{padding:14px}

    .cards{
      display:grid;
      grid-template-columns: 1fr 1fr;
      gap:14px;
    }
    @media (max-width: 760px){
      .cards{grid-template-columns:1fr}
    }

    .card{
      background:var(--panel2);
      border:1px solid var(--line);
      border-radius:var(--radius);
      padding:14px;
    }
    .card-top{display:flex;align-items:flex-start;justify-content:space-between;gap:10px;margin-bottom:10px}
    .node-name{font-weight:1000;font-size:14px}
    .badge{
      display:inline-flex;align-items:center;gap:8px;
      padding:6px 10px;border-radius:999px;
      background:var(--chip);border:1px solid var(--line);
      font-size:12px;font-weight:800;color:var(--text);
      white-space:nowrap;
    }
    .badge small{font-weight:800;color:var(--muted)}
    .fine{font-size:12px;color:var(--muted);margin-top:6px}

    .kv{
      display:grid;
      grid-template-columns: 1fr 1fr 1fr;
      gap:10px;
      margin-top:10px;
    }
    @media (max-width: 980px){
      .kv{grid-template-columns: 1fr 1fr}
    }
    .item{
      padding:10px;
      border:1px solid var(--line);
      border-radius:14px;
      background:rgba(255,255,255,.02);
    }
    .k{font-size:12px;color:var(--muted);margin-bottom:4px}
    .v{font-size:18px;font-weight:1000;letter-spacing:.2px}
    .v .unit{font-size:12px;font-weight:900;color:var(--muted);margin-left:6px}

    .split{
      display:grid;
      grid-template-columns: 1.2fr .8fr;
      gap:14px;
    }
    @media (max-width: 920px){
      .split{grid-template-columns:1fr}
    }

    /* Link stats */
    .stats{
      margin-top:10px;
      border-top:1px solid var(--line);
      padding-top:10px;
      display:grid;
      grid-template-columns: 1fr 1fr 1fr;
      gap:10px;
    }
    @media (max-width: 980px){
      .stats{grid-template-columns:1fr 1fr}
    }
    .stat{padding:10px;border:1px solid var(--line);border-radius:14px;background:rgba(255,255,255,.02)}
    .stat .k{margin:0 0 6px}
    .stat .v{font-size:14px;font-weight:950}
    .warn{color:var(--warn);font-weight:900}
    .bad{color:var(--bad);font-weight:900}
    .good{color:var(--good);font-weight:900}

    /* Charts */
    .chart-grid{
      display:grid;
      grid-template-columns: 1fr;
      gap:14px;
    }
    canvas{
      width:100%;
      height:230px;
      display:block;
      background:rgba(255,255,255,.02);
      border:1px solid var(--line);
      border-radius:14px;
    }
    .legend{display:flex;gap:10px;flex-wrap:wrap;margin-top:10px;color:var(--muted);font-size:12px}
    .lg{display:inline-flex;align-items:center;gap:8px}
    .sw{width:14px;height:4px;border-radius:999px;background:var(--n1)}
    .sw2{width:14px;height:4px;border-radius:999px;background:var(--n2)}
  </style>
</head>

<body>
  <div class="wrap">
    <header>
      <div>
        <h1>Receiver Dashboard</h1>
        <div class="sub">
          <span>AP:</span> <code>Receiver-Dashboard</code>
          <span style="margin-left:10px">IP:</span> <code>192.168.4.1</code>
        </div>
      </div>

      <div class="status">
        <div class="pill bad" id="pill"><span class="dot"></span><span id="st">Offline</span></div>
        <div class="meta">
          <div>Uptime: <span id="upt">-</span></div>
          <div>Refresh: <span id="last">-</span></div>
        </div>
      </div>
    </header>

    <div class="split">

      <div class="panel">
        <div class="hd">
          <div class="t">Nodes</div>
          <div class="meta">PM charts are PM1 / PM2.5 / PM10 • node colors match graphs</div>
        </div>
        <div class="bd">
          <div class="cards">

            <!-- Node 1 -->
            <div class="card">
              <div class="card-top">
                <div>
                  <div class="node-name">Node 1 <span style="color:var(--n1)">●</span></div>
                  <div class="fine" id="n1_note">—</div>
                </div>
                <div class="badge" id="n1_badge">—</div>
              </div>

              <div class="kv">
                <div class="item">
                  <div class="k">PM1</div>
                  <div class="v"><span id="n1_pm1">-</span><span class="unit">µg/m³</span></div>
                </div>
                <div class="item">
                  <div class="k">PM2.5</div>
                  <div class="v"><span id="n1_pm25">-</span><span class="unit">µg/m³</span></div>
                </div>
                <div class="item">
                  <div class="k">PM10</div>
                  <div class="v"><span id="n1_pm10">-</span><span class="unit">µg/m³</span></div>
                </div>
                <div class="item">
                  <div class="k">Temp</div>
                  <div class="v"><span id="n1_temp">-</span><span class="unit">°C</span></div>
                </div>
                <div class="item">
                  <div class="k">Humidity</div>
                  <div class="v"><span id="n1_hum">-</span><span class="unit">%</span></div>
                </div>
                <div class="item">
                  <div class="k">RX age</div>
                  <div class="v"><span id="n1_age">-</span><span class="unit">s</span></div>
                </div>
              </div>

              <div class="stats">
                <div class="stat">
                  <div class="k">Seq</div>
                  <div class="v" id="n1_seq">-</div>
                </div>
                <div class="stat">
                  <div class="k">PM age</div>
                  <div class="v"><span id="n1_pm_age">-</span>s</div>
                </div>
                <div class="stat">
                  <div class="k">Last ok</div>
                  <div class="v" id="n1_ok">-</div>
                </div>
                <div class="stat">
                  <div class="k">RX packets</div>
                  <div class="v" id="n1_rx">-</div>
                </div>
                <div class="stat">
                  <div class="k">Missed (est.)</div>
                  <div class="v" id="n1_miss">-</div>
                </div>
                <div class="stat">
                  <div class="k">Loss %</div>
                  <div class="v" id="n1_loss">-</div>
                </div>
              </div>
            </div>

            <!-- Node 2 -->
            <div class="card">
              <div class="card-top">
                <div>
                  <div class="node-name">Node 2 <span style="color:var(--n2)">●</span></div>
                  <div class="fine" id="n2_note">—</div>
                </div>
                <div class="badge" id="n2_badge">—</div>
              </div>

              <div class="kv">
                <div class="item">
                  <div class="k">PM1</div>
                  <div class="v"><span id="n2_pm1">-</span><span class="unit">µg/m³</span></div>
                </div>
                <div class="item">
                  <div class="k">PM2.5</div>
                  <div class="v"><span id="n2_pm25">-</span><span class="unit">µg/m³</span></div>
                </div>
                <div class="item">
                  <div class="k">PM10</div>
                  <div class="v"><span id="n2_pm10">-</span><span class="unit">µg/m³</span></div>
                </div>
                <div class="item">
                  <div class="k">Temp</div>
                  <div class="v"><span id="n2_temp">-</span><span class="unit">°C</span></div>
                </div>
                <div class="item">
                  <div class="k">Humidity</div>
                  <div class="v"><span id="n2_hum">-</span><span class="unit">%</span></div>
                </div>
                <div class="item">
                  <div class="k">RX age</div>
                  <div class="v"><span id="n2_age">-</span><span class="unit">s</span></div>
                </div>
              </div>

              <div class="stats">
                <div class="stat">
                  <div class="k">Seq</div>
                  <div class="v" id="n2_seq">-</div>
                </div>
                <div class="stat">
                  <div class="k">PM age</div>
                  <div class="v"><span id="n2_pm_age">-</span>s</div>
                </div>
                <div class="stat">
                  <div class="k">Last ok</div>
                  <div class="v" id="n2_ok">-</div>
                </div>
                <div class="stat">
                  <div class="k">RX packets</div>
                  <div class="v" id="n2_rx">-</div>
                </div>
                <div class="stat">
                  <div class="k">Missed (est.)</div>
                  <div class="v" id="n2_miss">-</div>
                </div>
                <div class="stat">
                  <div class="k">Loss %</div>
                  <div class="v" id="n2_loss">-</div>
                </div>
              </div>
            </div>

          </div><!-- cards -->
        </div>
      </div>

      <div class="panel">
        <div class="hd">
          <div class="t">PM History</div>
          <div class="meta">Rolling window • 1 sample/sec • stale data still shown</div>
        </div>
        <div class="bd">
          <div class="chart-grid">
            <div>
              <div class="sub" style="margin:0 0 8px;color:var(--muted)">PM1</div>
              <canvas id="c_pm1"></canvas>
              <div class="legend"><div class="lg"><span class="sw"></span> Node 1</div><div class="lg"><span class="sw2"></span> Node 2</div></div>
            </div>
            <div>
              <div class="sub" style="margin:0 0 8px;color:var(--muted)">PM2.5</div>
              <canvas id="c_pm25"></canvas>
              <div class="legend"><div class="lg"><span class="sw"></span> Node 1</div><div class="lg"><span class="sw2"></span> Node 2</div></div>
            </div>
            <div>
              <div class="sub" style="margin:0 0 8px;color:var(--muted)">PM10</div>
              <canvas id="c_pm10"></canvas>
              <div class="legend"><div class="lg"><span class="sw"></span> Node 1</div><div class="lg"><span class="sw2"></span> Node 2</div></div>
            </div>
          </div>
        </div>
      </div>

    </div><!-- split -->
  </div><!-- wrap -->

<script>
const $ = (id) => document.getElementById(id);
function fmt(x, d=1){ return (typeof x === "number" && isFinite(x)) ? x.toFixed(d) : "-"; }

function setPill(state){
  const pill = $("pill");
  pill.classList.remove("good","warn","bad");
  if(state==="good"){ pill.classList.add("good"); $("st").textContent="Online"; }
  else if(state==="warn"){ pill.classList.add("warn"); $("st").textContent="Laggy"; }
  else { pill.classList.add("bad"); $("st").textContent="Offline"; }
}

function setBadge(n, present, age_s){
  const el = $(`n${n}_badge`);
  if(!present){
    el.innerHTML = `Status: <small class="bad">absent</small>`;
    return;
  }
  if(age_s > 3){
    el.innerHTML = `Status: <small class="warn">stale</small>`;
    return;
  }
  el.innerHTML = `Status: <small class="good">live</small>`;
}

/* ====== Link stats (estimated loss from seq gaps) ====== */
const link = {
  1:{ lastSeq:null, rx:0, miss:0 },
  2:{ lastSeq:null, rx:0, miss:0 }
};

function updateLoss(n, seq){
  const s = link[n];
  if(typeof seq !== "number" || !isFinite(seq)) return;

  if(s.lastSeq !== null){
    const d = seq - s.lastSeq;
    if(d > 1) s.miss += (d - 1);
    // if d <= 0, ignore (reboot or wrap)
  }
  s.lastSeq = seq;
  s.rx += 1;

  const total = s.rx + s.miss;
  const lossPct = total ? (100 * s.miss / total) : 0;

  $(`n${n}_rx`).textContent = s.rx.toString();
  $(`n${n}_miss`).textContent = s.miss.toString();
  $(`n${n}_loss`).textContent = total ? lossPct.toFixed(1) + "%" : "-";
}

/* ====== Charts (no libs) ====== */
const HISTORY = 180;
const hist = {
  pm1:  { n1:[], n2:[] },
  pm25: { n1:[], n2:[] },
  pm10: { n1:[], n2:[] }
};

function pushHist(arr, v){
  arr.push(v);
  while(arr.length > HISTORY) arr.shift();
}

function setupCanvas(id){
  const c = $(id);
  const ctx = c.getContext("2d");
  function resize(){
    const dpr = window.devicePixelRatio || 1;
    const r = c.getBoundingClientRect();
    c.width  = Math.max(1, Math.floor(r.width * dpr));
    c.height = Math.max(1, Math.floor(r.height * dpr));
    ctx.setTransform(dpr,0,0,dpr,0,0);
  }
  resize();
  window.addEventListener("resize", resize);
  return { c, ctx, resize };
}

const cv1  = setupCanvas("c_pm1");
const cv25 = setupCanvas("c_pm25");
const cv10 = setupCanvas("c_pm10");

function drawChart(cv, series1, series2){
  const c = cv.c, ctx = cv.ctx;
  const w = c.getBoundingClientRect().width;
  const h = c.getBoundingClientRect().height;

  ctx.clearRect(0,0,w,h);

  const padL=42, padR=10, padT=10, padB=22;
  const plotW = w - padL - padR;
  const plotH = h - padT - padB;

  const line = getComputedStyle(document.documentElement).getPropertyValue("--line").trim() || "#263041";
  const muted = getComputedStyle(document.documentElement).getPropertyValue("--muted").trim() || "#a7b2c2";

  const all=[];
  for(const v of series1) if(v!=null && isFinite(v)) all.push(v);
  for(const v of series2) if(v!=null && isFinite(v)) all.push(v);
  let maxV = all.length ? Math.max(...all) : 50;
  maxV = Math.max(10, maxV * 1.15);

  ctx.strokeStyle=line; ctx.lineWidth=1;
  ctx.fillStyle=muted; ctx.font="12px system-ui";

  const steps=5;
  for(let i=0;i<=steps;i++){
    const y = padT + plotH*i/steps;
    ctx.beginPath(); ctx.moveTo(padL,y); ctx.lineTo(padL+plotW,y); ctx.stroke();
    const val = maxV*(1 - i/steps);
    ctx.fillText(Math.round(val).toString(), 6, y+4);
  }

  function xFor(i,len){ return (len<=1)?padL:(padL + plotW*i/(len-1)); }
  function yFor(v){ return padT + plotH*(1 - (v/maxV)); }

  function plot(arr,color){
    ctx.strokeStyle=color; ctx.lineWidth=2;
    ctx.beginPath();
    let started=false;
    for(let i=0;i<arr.length;i++){
      const v=arr[i];
      if(v==null || !isFinite(v)){ started=false; continue; }
      const x=xFor(i,arr.length), y=yFor(v);
      if(!started){ ctx.moveTo(x,y); started=true; }
      else ctx.lineTo(x,y);
    }
    ctx.stroke();
  }

  plot(series1, getComputedStyle(document.documentElement).getPropertyValue("--n1").trim() || "#4f8cff");
  plot(series2, getComputedStyle(document.documentElement).getPropertyValue("--n2").trim() || "#40d17a");

  ctx.fillText(`${HISTORY}s window`, padL, h-6);
}

/* ====== UI update ====== */
function setNode(n, d){
  const present = !!d.present;
  const age_s = present ? (d.age_s || 0) : 0;

  setBadge(n, present, age_s);

  $(`n${n}_age`).textContent = present ? fmt(age_s,1) : "-";
  $(`n${n}_seq`).textContent = present ? (d.seq ?? "-") : "-";

  // ok flag from latest packet (not PM validity)
  $(`n${n}_ok`).textContent = present ? ((d.last_ok ? "1" : "0")) : "-";

  const pmShow = present && !!d.pm_valid;
  $(`n${n}_pm1`).textContent  = pmShow ? d.pm1  : "-";
  $(`n${n}_pm25`).textContent = pmShow ? d.pm25 : "-";
  $(`n${n}_pm10`).textContent = pmShow ? d.pm10 : "-";

  const pmAge = pmShow ? (d.pm_age_s || 0) : null;
  $(`n${n}_pm_age`).textContent = pmShow ? fmt(pmAge,1) : "-";

  // BME: only temp/hum (pressure removed)
  if(present && d.hasBme){
    $(`n${n}_temp`).textContent = fmt(d.temp_c,2);
    $(`n${n}_hum`).textContent  = fmt(d.hum_pct,2);
  } else {
    $(`n${n}_temp`).textContent = "-";
    $(`n${n}_hum`).textContent  = "-";
  }

  const note = $(`n${n}_note`);
  if(!present) note.textContent = "No packets received yet.";
  else if(pmShow && pmAge > 3) note.textContent = `PM stale (${fmt(pmAge,1)}s) — holding last good.`;
  else note.textContent = "Receiving packets.";

  // Link stats (estimated loss from seq gaps)
  if(present) updateLoss(n, d.seq);

  // Push chart histories (use null if no PM)
  const v1  = pmShow ? d.pm1  : null;
  const v25 = pmShow ? d.pm25 : null;
  const v10 = pmShow ? d.pm10 : null;

  if(n===1){
    pushHist(hist.pm1.n1,  v1);
    pushHist(hist.pm25.n1, v25);
    pushHist(hist.pm10.n1, v10);
  } else {
    pushHist(hist.pm1.n2,  v1);
    pushHist(hist.pm25.n2, v25);
    pushHist(hist.pm10.n2, v10);
  }
}

async function tick(){
  try{
    const r = await fetch("/telemetry?ts=" + Date.now(), {cache:"no-store"});
    const j = await r.json();

    $("upt").textContent = Math.round((j.uptime_ms||0)/1000) + "s";
    $("last").textContent = new Date().toLocaleTimeString();

    setNode(1, j.node1 || {});
    setNode(2, j.node2 || {});

    const n1 = j.node1 || {};
    const n2 = j.node2 || {};
    const live1 = n1.present && (n1.age_s || 999) <= 3;
    const live2 = n2.present && (n2.age_s || 999) <= 3;
    const anyPresent = (n1.present || n2.present);

    if(live1 || live2) setPill("good");
    else if(anyPresent) setPill("warn");
    else setPill("bad");

    drawChart(cv1,  hist.pm1.n1,  hist.pm1.n2);
    drawChart(cv25, hist.pm25.n1, hist.pm25.n2);
    drawChart(cv10, hist.pm10.n1, hist.pm10.n2);

  }catch(e){
    setPill("bad");
  }
}

setInterval(tick, 1000);
tick();
</script>
</body>
</html>
)HTML";