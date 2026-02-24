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
      --bg:#0b0d10; --panel:#12161c; --panel2:#0f1318; --text:#e9eef6; --muted:#a7b2c2;
      --line:#263041; --good:#40d17a; --warn:#f6c453; --bad:#ff5a6a; --chip:#1b2230;
      --shadow: 0 10px 30px rgba(0,0,0,.35); --radius:16px; --n1:#4f8cff; --n2:#40d17a;
    }
    @media (prefers-color-scheme: light){
      :root{
        --bg:#f6f8fb; --panel:#ffffff; --panel2:#fbfcfe; --text:#0f172a; --muted:#475569;
        --line:#e2e8f0; --chip:#eef2f7; --shadow: 0 12px 26px rgba(2,6,23,.10);
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
    header{display:flex;gap:14px;align-items:center;justify-content:space-between;flex-wrap:wrap;padding:10px 4px 10px}
    h1{margin:0;font-size:20px;letter-spacing:.2px}
    .sub{color:var(--muted);font-size:13px;line-height:1.35}
    code{background:var(--chip);padding:2px 8px;border-radius:10px;border:1px solid var(--line)}
    .status{display:flex;align-items:center;gap:10px;padding:10px 12px;border:1px solid var(--line);border-radius:999px;background:rgba(255,255,255,.02);box-shadow:var(--shadow);min-width:260px;justify-content:space-between}
    .pill{display:inline-flex;align-items:center;gap:8px;padding:6px 10px;border-radius:999px;font-weight:900;font-size:12px;background:var(--chip);border:1px solid var(--line)}
    .dot{width:10px;height:10px;border-radius:50%}
    .pill.good .dot{background:var(--good)} .pill.bad .dot{background:var(--bad)} .pill.warn .dot{background:var(--warn)}
    .meta{font-size:12px;color:var(--muted)} .meta div{white-space:nowrap}

    /* ALERT */
    .alert{
      margin:10px 4px 14px;
      border:1px solid var(--line);
      border-radius:var(--radius);
      box-shadow: var(--shadow);
      padding:12px 14px;
      display:flex; align-items:center; justify-content:space-between; gap:12px;
      background:rgba(255,255,255,.02);
    }
    .alertL{display:flex;align-items:center;gap:10px;min-width:0}
    .badge{display:inline-flex;align-items:center;gap:8px;padding:6px 10px;border-radius:999px;background:var(--chip);border:1px solid var(--line);font-size:12px;font-weight:900;white-space:nowrap}
    .badge.good{border-color:rgba(64,209,122,.35)} .badge.warn{border-color:rgba(246,196,83,.35)} .badge.bad{border-color:rgba(255,90,106,.35)}
    .ico{width:10px;height:10px;border-radius:50%}
    .badge.good .ico{background:var(--good)} .badge.warn .ico{background:var(--warn)} .badge.bad .ico{background:var(--bad)}
    .alertT{font-weight:1000;letter-spacing:.2px}
    .alertD{color:var(--muted);font-size:12px;line-height:1.35;min-width:0;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}

    .panel{background:linear-gradient(180deg, rgba(255,255,255,.04), rgba(255,255,255,.00));background-color:var(--panel);border:1px solid var(--line);border-radius:var(--radius);box-shadow:var(--shadow);overflow:hidden}
    .hd{display:flex;align-items:center;justify-content:space-between;gap:10px;padding:14px 14px 10px;border-bottom:1px solid var(--line);background:linear-gradient(180deg, rgba(255,255,255,.04), transparent)}
    .hd .t{font-size:14px;font-weight:1000;letter-spacing:.2px}
    .bd{padding:14px}

    .split{display:grid;grid-template-columns:1.2fr .8fr;gap:14px}
    @media (max-width:920px){.split{grid-template-columns:1fr}}

    .cards{display:grid;grid-template-columns:1fr 1fr;gap:14px}
    @media (max-width:760px){.cards{grid-template-columns:1fr}}
    .card{background:var(--panel2);border:1px solid var(--line);border-radius:var(--radius);padding:14px}
    .card-top{display:flex;align-items:flex-start;justify-content:space-between;gap:10px;margin-bottom:10px}
    .node-name{font-weight:1000;font-size:14px}
    .fine{font-size:12px;color:var(--muted);margin-top:6px}

    .kv{display:grid;grid-template-columns:1fr 1fr 1fr;gap:10px;margin-top:10px}
    @media (max-width:980px){.kv{grid-template-columns:1fr 1fr}}
    .item{padding:10px;border:1px solid var(--line);border-radius:14px;background:rgba(255,255,255,.02)}
    .k{font-size:12px;color:var(--muted);margin-bottom:4px}
    .v{font-size:18px;font-weight:1000;letter-spacing:.2px}
    .v .unit{font-size:12px;font-weight:900;color:var(--muted);margin-left:6px}

    .stats{margin-top:10px;border-top:1px solid var(--line);padding-top:10px;display:grid;grid-template-columns:1fr 1fr 1fr;gap:10px}
    @media (max-width:980px){.stats{grid-template-columns:1fr 1fr}}
    .stat{padding:10px;border:1px solid var(--line);border-radius:14px;background:rgba(255,255,255,.02)}
    .stat .k{margin:0 0 6px} .stat .v{font-size:14px;font-weight:950}

    .chart-grid{display:grid;grid-template-columns:1fr;gap:14px}
    canvas{width:100%;height:210px;display:block;background:rgba(255,255,255,.02);border:1px solid var(--line);border-radius:14px}
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

    <!-- ALERT -->
    <div class="alert">
      <div class="alertL">
        <div class="badge good" id="a_badge"><span class="ico"></span><span id="a_title">Normal</span></div>
        <div style="min-width:0">
          <div class="alertT" id="a_line">No smoke conditions detected.</div>
          <div class="alertD" id="a_desc">Heuristic alert. Tune thresholds after baseline + fire pit tests.</div>
        </div>
      </div>
      <div class="meta" id="a_meta">—</div>
    </div>

    <div class="split">
      <div class="panel">
        <div class="hd">
          <div class="t">Nodes</div>
          <div class="meta">PM + Temp/Humidity graphs • node colors match lines</div>
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
                <div class="item"><div class="k">PM1</div><div class="v"><span id="n1_pm1">-</span><span class="unit">µg/m³</span></div></div>
                <div class="item"><div class="k">PM2.5</div><div class="v"><span id="n1_pm25">-</span><span class="unit">µg/m³</span></div></div>
                <div class="item"><div class="k">PM10</div><div class="v"><span id="n1_pm10">-</span><span class="unit">µg/m³</span></div></div>
                <div class="item"><div class="k">Temp</div><div class="v"><span id="n1_temp">-</span><span class="unit">°C</span></div></div>
                <div class="item"><div class="k">Humidity</div><div class="v"><span id="n1_hum">-</span><span class="unit">%</span></div></div>
                <div class="item"><div class="k">RX age</div><div class="v"><span id="n1_age">-</span><span class="unit">s</span></div></div>
              </div>

              <div class="stats">
                <div class="stat"><div class="k">Seq</div><div class="v" id="n1_seq">-</div></div>
                <div class="stat"><div class="k">PM age</div><div class="v"><span id="n1_pm_age">-</span>s</div></div>
                <div class="stat"><div class="k">Last ok</div><div class="v" id="n1_ok">-</div></div>
                <div class="stat"><div class="k">RX packets</div><div class="v" id="n1_rx">-</div></div>
                <div class="stat"><div class="k">Missed (est.)</div><div class="v" id="n1_miss">-</div></div>
                <div class="stat"><div class="k">Loss %</div><div class="v" id="n1_loss">-</div></div>
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
                <div class="item"><div class="k">PM1</div><div class="v"><span id="n2_pm1">-</span><span class="unit">µg/m³</span></div></div>
                <div class="item"><div class="k">PM2.5</div><div class="v"><span id="n2_pm25">-</span><span class="unit">µg/m³</span></div></div>
                <div class="item"><div class="k">PM10</div><div class="v"><span id="n2_pm10">-</span><span class="unit">µg/m³</span></div></div>
                <div class="item"><div class="k">Temp</div><div class="v"><span id="n2_temp">-</span><span class="unit">°C</span></div></div>
                <div class="item"><div class="k">Humidity</div><div class="v"><span id="n2_hum">-</span><span class="unit">%</span></div></div>
                <div class="item"><div class="k">RX age</div><div class="v"><span id="n2_age">-</span><span class="unit">s</span></div></div>
              </div>

              <div class="stats">
                <div class="stat"><div class="k">Seq</div><div class="v" id="n2_seq">-</div></div>
                <div class="stat"><div class="k">PM age</div><div class="v"><span id="n2_pm_age">-</span>s</div></div>
                <div class="stat"><div class="k">Last ok</div><div class="v" id="n2_ok">-</div></div>
                <div class="stat"><div class="k">RX packets</div><div class="v" id="n2_rx">-</div></div>
                <div class="stat"><div class="k">Missed (est.)</div><div class="v" id="n2_miss">-</div></div>
                <div class="stat"><div class="k">Loss %</div><div class="v" id="n2_loss">-</div></div>
              </div>
            </div>

          </div>
        </div>
      </div>

      <div class="panel">
        <div class="hd">
          <div class="t">History</div>
          <div class="meta">Rolling window • 1 sample/sec</div>
        </div>
        <div class="bd">
          <div class="chart-grid">
            <div><div class="sub" style="margin:0 0 8px;color:var(--muted)">PM1</div><canvas id="c_pm1"></canvas><div class="legend"><div class="lg"><span class="sw"></span> Node 1</div><div class="lg"><span class="sw2"></span> Node 2</div></div></div>
            <div><div class="sub" style="margin:0 0 8px;color:var(--muted)">PM2.5</div><canvas id="c_pm25"></canvas><div class="legend"><div class="lg"><span class="sw"></span> Node 1</div><div class="lg"><span class="sw2"></span> Node 2</div></div></div>
            <div><div class="sub" style="margin:0 0 8px;color:var(--muted)">PM10</div><canvas id="c_pm10"></canvas><div class="legend"><div class="lg"><span class="sw"></span> Node 1</div><div class="lg"><span class="sw2"></span> Node 2</div></div></div>
            <div><div class="sub" style="margin:0 0 8px;color:var(--muted)">Temperature</div><canvas id="c_temp"></canvas><div class="legend"><div class="lg"><span class="sw"></span> Node 1</div><div class="lg"><span class="sw2"></span> Node 2</div></div></div>
            <div><div class="sub" style="margin:0 0 8px;color:var(--muted)">Humidity</div><canvas id="c_hum"></canvas><div class="legend"><div class="lg"><span class="sw"></span> Node 1</div><div class="lg"><span class="sw2"></span> Node 2</div></div></div>
          </div>
        </div>
      </div>

    </div>
  </div>

<script>
const $ = (id) => document.getElementById(id);
function fmt(x,d=1){ return (typeof x==="number" && isFinite(x)) ? x.toFixed(d) : "-"; }

function setPill(state){
  const pill = $("pill");
  pill.classList.remove("good","warn","bad");
  if(state==="good"){ pill.classList.add("good"); $("st").textContent="Online"; }
  else if(state==="warn"){ pill.classList.add("warn"); $("st").textContent="Laggy"; }
  else { pill.classList.add("bad"); $("st").textContent="Offline"; }
}
function setBadge(n,present,age_s){
  const el = $(`n${n}_badge`);
  if(!present){ el.textContent="Status: absent"; return; }
  if(age_s>3){ el.textContent="Status: stale"; return; }
  el.textContent="Status: live";
}

/* Loss estimate from seq gaps (valid only if sender seq increases ~1/sec) */
const link={1:{lastSeq:null,rx:0,miss:0},2:{lastSeq:null,rx:0,miss:0}};
function updateLoss(n,seq){
  const s=link[n];
  if(typeof seq!=="number"||!isFinite(seq)) return;
  if(s.lastSeq!==null){
    const d=seq-s.lastSeq;
    if(d>1) s.miss+=(d-1);
  }
  s.lastSeq=seq; s.rx++;
  const total=s.rx+s.miss;
  $(`n${n}_rx`).textContent=s.rx.toString();
  $(`n${n}_miss`).textContent=s.miss.toString();
  $(`n${n}_loss`).textContent=total? (100*s.miss/total).toFixed(1)+"%":"-";
}

/* Charts */
const HISTORY=180;
const hist={
  pm1:{n1:[],n2:[]}, pm25:{n1:[],n2:[]}, pm10:{n1:[],n2:[]},
  temp:{n1:[],n2:[]}, hum:{n1:[],n2:[]}
};
function pushHist(arr,v){ arr.push(v); while(arr.length>HISTORY) arr.shift(); }

function setupCanvas(id){
  const c=$(id), ctx=c.getContext("2d");
  function resize(){
    const dpr=window.devicePixelRatio||1;
    const r=c.getBoundingClientRect();
    c.width=Math.max(1,Math.floor(r.width*dpr));
    c.height=Math.max(1,Math.floor(r.height*dpr));
    ctx.setTransform(dpr,0,0,dpr,0,0);
  }
  resize(); window.addEventListener("resize", resize);
  return {c,ctx};
}
const cv_pm1=setupCanvas("c_pm1"), cv_pm25=setupCanvas("c_pm25"), cv_pm10=setupCanvas("c_pm10");
const cv_temp=setupCanvas("c_temp"), cv_hum=setupCanvas("c_hum");

function drawChart(cv,a,b,forceMin=null,forceMax=null){
  const c=cv.c, ctx=cv.ctx;
  const w=c.getBoundingClientRect().width, h=c.getBoundingClientRect().height;
  ctx.clearRect(0,0,w,h);
  const padL=42,padR=10,padT=10,padB=22;
  const plotW=w-padL-padR, plotH=h-padT-padB;

  const line=getComputedStyle(document.documentElement).getPropertyValue("--line").trim()||"#263041";
  const muted=getComputedStyle(document.documentElement).getPropertyValue("--muted").trim()||"#a7b2c2";
  const n1c=getComputedStyle(document.documentElement).getPropertyValue("--n1").trim()||"#4f8cff";
  const n2c=getComputedStyle(document.documentElement).getPropertyValue("--n2").trim()||"#40d17a";

  const all=[];
  for(const v of a) if(v!=null&&isFinite(v)) all.push(v);
  for(const v of b) if(v!=null&&isFinite(v)) all.push(v);
  let maxV=all.length?Math.max(...all):50;
  let minV=all.length?Math.min(...all):0;

  if(forceMin!==null) minV=forceMin;
  if(forceMax!==null) maxV=Math.max(forceMax,maxV);
  const span=Math.max(1e-6,(maxV-minV));
  maxV=maxV+span*0.15; minV=minV-span*0.05;

  ctx.strokeStyle=line; ctx.lineWidth=1; ctx.fillStyle=muted; ctx.font="12px system-ui";
  const steps=5;
  for(let i=0;i<=steps;i++){
    const y=padT+plotH*i/steps;
    ctx.beginPath(); ctx.moveTo(padL,y); ctx.lineTo(padL+plotW,y); ctx.stroke();
    const val=maxV-( (maxV-minV)*i/steps );
    ctx.fillText(Math.round(val).toString(), 6, y+4);
  }

  function xFor(i,len){ return (len<=1)?padL:(padL+plotW*i/(len-1)); }
  function yFor(v){ return padT+plotH*(1-((v-minV)/(maxV-minV))); }

  function plot(arr,color){
    ctx.strokeStyle=color; ctx.lineWidth=2;
    ctx.beginPath(); let started=false;
    for(let i=0;i<arr.length;i++){
      const v=arr[i];
      if(v==null||!isFinite(v)){ started=false; continue; }
      const x=xFor(i,arr.length), y=yFor(v);
      if(!started){ ctx.moveTo(x,y); started=true; } else ctx.lineTo(x,y);
    }
    ctx.stroke();
  }
  plot(a,n1c); plot(b,n2c);
  ctx.fillText(`${HISTORY}s window`, padL, h-6);
}

/* Alert logic (simple heuristic; tune with tests) */
const ALERT={
  windowS:30, minValid:15,
  smokePM25:60, firePM25:120,
  dryRH:35, warmC:28,
  requireBoth:true
};
function lastN(arr,n){ return arr.length<=n?arr.slice():arr.slice(arr.length-n); }
function avgValid(arr){
  let sum=0,c=0;
  for(const v of arr){ if(v==null||!isFinite(v)) continue; sum+=v; c++; }
  return {avg:c?sum/c:null, cnt:c};
}
function classifyNode(n, node){
  const pmArr = (n===1)?hist.pm25.n1:hist.pm25.n2;
  const tArr  = (n===1)?hist.temp.n1:hist.temp.n2;
  const hArr  = (n===1)?hist.hum.n1 :hist.hum.n2;

  const W=Math.min(ALERT.windowS,HISTORY);
  const pm=avgValid(lastN(pmArr,W));
  const tt=avgValid(lastN(tArr,W));
  const hh=avgValid(lastN(hArr,W));

  const present=!!node.present;
  const pmFresh = present && !!node.pm_valid && (node.pm_age_s!=null) && node.pm_age_s<=3;

  if(!pmFresh || pm.cnt<ALERT.minValid) return {smoke:false,fire:false,pm25:pm.avg,t:tt.avg,rh:hh.avg};

  const smoke = (pm.avg>=ALERT.smokePM25);
  const fire  = (pm.avg>=ALERT.firePM25) && (hh.avg!=null && hh.avg<=ALERT.dryRH) && (tt.avg!=null && tt.avg>=ALERT.warmC);
  return {smoke,fire,pm25:pm.avg,t:tt.avg,rh:hh.avg};
}
function setAlert(level,title,line,desc,meta){
  const b=$("a_badge");
  b.classList.remove("good","warn","bad");
  b.classList.add(level);
  $("a_title").textContent=title;
  $("a_line").textContent=line;
  $("a_desc").textContent=desc;
  $("a_meta").textContent=meta||"—";
}
function updateAlert(j){
  const n1=j.node1||{}, n2=j.node2||{};
  const c1=classifyNode(1,n1), c2=classifyNode(2,n2);

  const bothFire=c1.fire && c2.fire;
  const anyFire=c1.fire || c2.fire;
  const bothSmoke=c1.smoke && c2.smoke;
  const anySmoke=c1.smoke || c2.smoke;

  const meta=`30s avg PM2.5: N1 ${fmt(c1.pm25,1)} • N2 ${fmt(c2.pm25,1)} | RH: N1 ${fmt(c1.rh,1)}% • N2 ${fmt(c2.rh,1)}% | T: N1 ${fmt(c1.t,1)}°C • N2 ${fmt(c2.t,1)}°C`;

  if((ALERT.requireBoth && bothFire) || (!ALERT.requireBoth && anyFire)){
    setAlert("bad","Possible Wildfire","High PM2.5 + warm + dry sustained.","Not guaranteed. Validate with controlled smoke tests + multi-node agreement.",meta);
    return;
  }
  if(ALERT.requireBoth && anyFire){
    setAlert("warn","Smoke Detected","One node matches wildfire-like conditions. Waiting for confirmation.","If the other node agrees for ~30s, escalate.",meta);
    return;
  }
  if(anySmoke){
    setAlert("warn","Smoke Detected", bothSmoke ? "Both nodes show sustained elevated PM2.5." : "One node shows sustained elevated PM2.5.",
             "Could be smoke or dust. Use PM10 + placement to reduce false positives.",meta);
    return;
  }
  setAlert("good","Normal","No smoke conditions detected.","Tune thresholds after baseline + fire pit tests.",meta);
}

/* Main update */
function setNode(n,d){
  const present=!!d.present;
  const age_s=present?(d.age_s||0):0;
  setBadge(n,present,age_s);

  $(`n${n}_age`).textContent=present?fmt(age_s,1):"-";
  $(`n${n}_seq`).textContent=present?(d.seq??"-"):"-";
  $(`n${n}_ok`).textContent =present?(d.last_ok?"1":"0"):"-";

  const pmShow=present && !!d.pm_valid;
  $(`n${n}_pm1`).textContent =pmShow?d.pm1:"-";
  $(`n${n}_pm25`).textContent=pmShow?d.pm25:"-";
  $(`n${n}_pm10`).textContent=pmShow?d.pm10:"-";
  $(`n${n}_pm_age`).textContent=pmShow?fmt(d.pm_age_s||0,1):"-";

  const hasBme=present && !!d.hasBme;
  $(`n${n}_temp`).textContent=hasBme?fmt(d.temp_c,2):"-";
  $(`n${n}_hum`).textContent =hasBme?fmt(d.hum_pct,2):"-";

  const note=$(`n${n}_note`);
  if(!present) note.textContent="No packets received yet.";
  else if(pmShow && (d.pm_age_s||0)>3) note.textContent=`PM stale (${fmt(d.pm_age_s,1)}s) — holding last good.`;
  else note.textContent="Receiving packets.";

  if(present) updateLoss(n,d.seq);

  const v1=pmShow?d.pm1:null, v25=pmShow?d.pm25:null, v10=pmShow?d.pm10:null;
  const t=hasBme?d.temp_c:null, h=hasBme?d.hum_pct:null;

  if(n===1){
    pushHist(hist.pm1.n1,v1); pushHist(hist.pm25.n1,v25); pushHist(hist.pm10.n1,v10);
    pushHist(hist.temp.n1,t); pushHist(hist.hum.n1,h);
  }else{
    pushHist(hist.pm1.n2,v1); pushHist(hist.pm25.n2,v25); pushHist(hist.pm10.n2,v10);
    pushHist(hist.temp.n2,t); pushHist(hist.hum.n2,h);
  }
}

async function tick(){
  try{
    const r=await fetch("/telemetry?ts="+Date.now(), {cache:"no-store"});
    const j=await r.json();
    $("upt").textContent=Math.round((j.uptime_ms||0)/1000)+"s";
    $("last").textContent=new Date().toLocaleTimeString();

    setNode(1,j.node1||{}); setNode(2,j.node2||{});

    const n1=j.node1||{}, n2=j.node2||{};
    const live1=n1.present && (n1.age_s||999)<=3;
    const live2=n2.present && (n2.age_s||999)<=3;
    const anyPresent=(n1.present||n2.present);
    if(live1||live2) setPill("good"); else if(anyPresent) setPill("warn"); else setPill("bad");

    drawChart(cv_pm1, hist.pm1.n1, hist.pm1.n2);
    drawChart(cv_pm25,hist.pm25.n1,hist.pm25.n2);
    drawChart(cv_pm10,hist.pm10.n1,hist.pm10.n2);
    drawChart(cv_temp,hist.temp.n1,hist.temp.n2, 0, 35);  // force temp scale
    drawChart(cv_hum, hist.hum.n1, hist.hum.n2, 0, 100);  // force RH scale

    updateAlert(j);
  }catch(e){
    setPill("bad");
    setAlert("good","Normal","No smoke conditions detected.","Telemetry fetch failed.", "—");
  }
}
setInterval(tick,1000);
tick();
</script>
</body>
</html>
)HTML";