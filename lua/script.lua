-- gerado: dados de sprites
FR={}
FR["c1_s"]={0,0}
FR["c1_a"]={40,0}
FR["c1_b"]={80,0}
FR["c2_s"]={120,0}
FR["c2_a"]={160,0}
FR["c2_b"]={200,0}
FR["c3_s"]={240,0}
FR["c3_a"]={280,0}
FR["c3_b"]={320,0}
FR["c4_s"]={360,0}
FR["c4_a"]={400,0}
FR["c4_b"]={440,0}
FR["c5_s"]={0,40}
FR["c5_a"]={40,40}
FR["c5_b"]={80,40}
FR["c6_s"]={120,40}
FR["c6_a"]={160,40}
FR["c6_b"]={200,40}
FR["c7_s"]={240,40}
FR["c7_a"]={280,40}
FR["c7_b"]={320,40}
FR["c8_s"]={360,40}
FR["c8_a"]={400,40}
FR["c8_b"]={440,40}
FR["gk_s"]={0,80}
FR["gk_a"]={40,80}
FR["gk_b"]={80,80}
FR["neymar_s"]={120,80}
FR["neymar_a"]={160,80}
FR["neymar_b"]={200,80}
FR["messi_s"]={240,80}
FR["messi_a"]={280,80}
FR["messi_b"]={320,80}
FR["pele_s"]={360,80}
FR["pele_a"]={400,80}
FR["pele_b"]={440,80}
FR["cr7_s"]={0,120}
FR["cr7_a"]={40,120}
FR["cr7_b"]={80,120}
FR["mara_s"]={120,120}
FR["mara_a"]={160,120}
FR["mara_b"]={200,120}
FR["ring_h"]={240,120}
FR["ring_a"]={280,120}
FR["arrow"]={320,120}
FR["ball"]={360,120}
FR["shadow"]={400,120}


-- ============================================================
--  SUPER PIXEL SOCCER PSP  (Lua Player)
--  Controles: direcional/analogico move | X passe | O chute
--  [] lob | R correr | START pausa
-- ============================================================
SW, SH = 480, 272
C = {
  white = Color.new(255,255,255), black = Color.new(0,0,0),
  yellow = Color.new(255,230,60), red = Color.new(255,90,90),
  green = Color.new(90,235,130), blue = Color.new(100,180,255),
  grey = Color.new(170,170,170), dim = Color.new(0,0,0,150),
  panel = Color.new(10,24,50,215), sel = Color.new(255,200,40,230),
  pitch = Color.new(30,110,40), orange = Color.new(255,160,40)
}
IMG, SND = {}, {}
OPT = { diff = 2, half = 2, sound = true }   -- half: minutos reais por tempo

function loadImg(name, path)
  local ok, res = pcall(function() return Image.load(path) end)
  if ok then IMG[name] = res else IMG[name] = nil end
end
function loadSnd(name, path)
  local ok, res = pcall(function() return Sound.load(path, false) end)
  if ok and res then SND[name] = res end
end
function play(name)
  if OPT.sound and SND[name] then pcall(function() SND[name]:play() end) end
end

function saveOpt()
  pcall(function()
    local f = io.open("save.txt", "w")
    if f then f:write(OPT.diff .. " " .. OPT.half .. " " .. (OPT.sound and 1 or 0)); f:close() end
  end)
end
function loadOpt()
  pcall(function()
    local f = io.open("save.txt", "r")
    if f then
      local s = f:read("*a"); f:close()
      local a, b, c = string.match(s or "", "(%d+) (%d+) (%d+)")
      if a then OPT.diff = tonumber(a); OPT.half = tonumber(b); OPT.sound = (tonumber(c) == 1) end
    end
  end)
end

-- ---------------- entrada ----------------
CUR, PRV = {}, {}
AX, AY = 0, 0
function readPad()
  PRV = CUR
  local p = Controls.read()
  CUR = {
    up = p:up(), down = p:down(), left = p:left(), right = p:right(),
    cross = p:cross(), circle = p:circle(), square = p:square(), triangle = p:triangle(),
    l = p:l(), r = p:r(), start = p:start(), select = p:select()
  }
  AX, AY = p:analogX(), p:analogY()
end
function hit(b) return CUR[b] and not PRV[b] end
function held(b) return CUR[b] end
function dirInput()
  local dx, dy = 0, 0
  if CUR.left then dx = dx - 1 end
  if CUR.right then dx = dx + 1 end
  if CUR.up then dy = dy - 1 end
  if CUR.down then dy = dy + 1 end
  if math.abs(AX) > 45 then dx = AX / 128 end
  if math.abs(AY) > 45 then dy = AY / 128 end
  local l = math.sqrt(dx * dx + dy * dy)
  if l > 1 then dx, dy = dx / l, dy / l end
  return dx, dy
end

-- ---------------- util ----------------
function clamp(v, a, b) if v < a then return a elseif v > b then return b end return v end
function rnd(a, b) return a + math.random() * (b - a) end
function dist(ax, ay, bx, by) local dx, dy = ax - bx, ay - by; return math.sqrt(dx * dx + dy * dy) end
function norm(dx, dy) local l = math.sqrt(dx * dx + dy * dy); if l < 0.0001 then return 0, 0, 0 end return dx / l, dy / l, l end

function txt(x, y, s, c)
  screen:print(x + 1, y + 1, s, C.black)
  screen:print(x, y, s, c or C.white)
end
function ctxt(y, s, c, cx) txt((cx or SW / 2) - #s * 4, y, s, c) end
function box(x, y, w, h, c) screen:fillRect(x, y, w, h, c or C.panel) end

function drawBG(name)
  if IMG[name] then screen:blit(0, 0, IMG[name]) else screen:clear(C.pitch) end
end
function frame(key, x, y)    -- desenha celula 40x40 do atlas com os pes em (x,y)
  local f = FR[key]
  if f and IMG.atlas then screen:blit(math.floor(x - 20), math.floor(y - 39), IMG.atlas, f[1], f[2], 40, 40) end
end

function flip()
  screen.waitVblankStart()
  screen.flip()
end

-- ---------------- times ----------------
-- kit: sprite base; str: forca 1..5
TEAMS = {
  { name = "TRICOLOR",  short = "TRI", kit = "c1", str = 4, col = Color.new(150,30,60) },
  { name = "VERDAO",    short = "VRD", kit = "c2", str = 5, col = Color.new(20,130,60) },
  { name = "CRUZ MALT", short = "CMA", kit = "c3", str = 3, col = Color.new(40,40,40) },
  { name = "PEIXE",     short = "PXE", kit = "c4", str = 4, col = Color.new(230,230,230) },
  { name = "MACACA",    short = "MAC", kit = "c5", str = 2, col = Color.new(220,220,220) },
  { name = "ALVINEGRO", short = "ALV", kit = "c6", str = 3, col = Color.new(90,90,90) },
  { name = "LEAO",      short = "LEA", kit = "c7", str = 3, col = Color.new(200,30,30) },
  { name = "ESMERALDA", short = "ESM", kit = "c8", str = 2, col = Color.new(30,150,80) },
  { name = "LENDAS",    short = "LEN", kit = "c4", kits = { "pele", "messi", "neymar", "cr7", "mara" }, str = 5, col = Color.new(255,215,60) }
}
NCLUBS = 8

-- formacoes: {rx, ry} no sentido do ataque (rx 0 = proprio gol)
FORM6 = { {0.04,0.5}, {0.22,0.24}, {0.22,0.76}, {0.45,0.5}, {0.68,0.3}, {0.68,0.7} }
FORM5 = { {0.04,0.5}, {0.24,0.3}, {0.24,0.7}, {0.6,0.3}, {0.6,0.7} }

-- ============================================================
--  MOTOR DA PARTIDA
-- ============================================================
FL, FW = 800, 420          -- comprimento e largura logicos do campo
GOAL_Y1, GOAL_Y2, GOAL_H = 150, 250, 30
GRAV = 0.12
M = nil

-- campo (logico) -> tela (perspectiva do fundo)
function toScreen(x, y)
  local u = x / FL
  local v = y / FW
  local f
  if v < 0 then f = v else f = v ^ 1.14 end
  local l = 58 - 50 * f
  local r = 422 + 50 * f
  return l + (r - l) * u, 70 + 174 * f
end

function atkX(ti) if M.dir[ti] == 1 then return FL else return 0 end end
function ownX(ti) if M.dir[ti] == 1 then return 0 else return FL end end

function homePos(p)
  local form = M.form[p.t]
  local f = form[p.i]
  local rx, ry = f[1], f[2]
  if M.dir[p.t] == 1 then return rx * FL, ry * FW end
  return (1 - rx) * FL, ry * FW
end

function newPlayer(ti, n, kit)
  return { t = ti, i = n, gk = (n == 1), x = 0, y = 0, vx = 0, vy = 0,
           fx = (ti == 1) and 1 or -1, fy = 0, kick = 0, stun = 0, tack = 0,
           anim = 0, hold = 0, kit = kit, moving = false }
end

function teamKit(ti, n)
  local tm = TEAMS[M.team[ti]]
  if n == 1 then return "gk" end
  if tm.kits then return tm.kits[((n - 2) % #tm.kits) + 1] end
  return tm.kit
end

function newMatch(o)
  M = {
    team = { o.t1, o.t2 }, futsal = o.futsal or false, training = o.training or false,
    score = { 0, 0 }, half = 1, hf = 0, dir = { 1, -1 },
    state = "kick", st = 0, players = {}, ctrl = nil, charge = 0, freeze = 0,
    onEnd = o.onEnd, bg = (o.futsal and "bg_court" or "bg_field"),
    msg = "", trainKit = o.trainKit, shots = { 0, 0 }, kickTeam = 1
  }
  local secs = ({ 45, 90, 150 })[OPT.half] or 90
  M.halfFrames = secs * 60
  if M.futsal then M.form = { FORM5, FORM5 } else M.form = { FORM6, FORM6 } end
  M.ball = { x = FL / 2, y = FW / 2, z = 0, vx = 0, vy = 0, vz = 0, owner = nil, last = 1 }
  for ti = 1, 2 do
    local list = {}
    for n = 1, #M.form[ti] do list[#list + 1] = n end
    if M.training then
      if ti == 1 then list = { 4 } else list = { 1, 2, 3 } end
    end
    for _, n in ipairs(list) do
      local kit = teamKit(ti, n)
      if M.training and ti == 1 then kit = M.trainKit or "neymar" end
      M.players[#M.players + 1] = newPlayer(ti, n, kit)
    end
  end
  startKickoff(1)
end

function startKickoff(ti)
  M.kickTeam = ti
  M.state = "kick"; M.st = 80
  local b = M.ball
  b.x, b.y, b.z, b.vx, b.vy, b.vz, b.owner = FL / 2, FW / 2, 0, 0, 0, 0, nil
  local kicker = nil
  for _, p in ipairs(M.players) do
    local hx, hy = homePos(p)
    local d = M.dir[p.t]
    if M.training then
      p.x, p.y = hx, hy
    else
      p.x = FL / 2 + (hx - FL / 2) * 0.7
      p.y = hy
    end
    p.vx, p.vy, p.stun, p.tack, p.kick, p.hold = 0, 0, 0, 0, 0, 0
    p.fx, p.fy = d, 0
    if p.t == ti and p.i == 4 then kicker = p end
  end
  if kicker then
    kicker.x, kicker.y = FL / 2 - M.dir[ti] * 4, FW / 2
    b.owner = kicker; b.last = ti
  end
  if M.training then
    local h = nil
    for _, p in ipairs(M.players) do if p.t == 1 then h = p end end
    M.ctrl = h
    if h then h.x, h.y = 300, FW / 2; b.x, b.y = 310, FW / 2; b.owner = nil end
  else
    M.ctrl = nearestField(1, b.x, b.y)
  end
end

function nearestField(ti, x, y, skip)
  local best, bd = nil, 1e9
  for _, p in ipairs(M.players) do
    if p.t == ti and not p.gk and p ~= skip then
      local d = dist(p.x, p.y, x, y)
      if d < bd then best, bd = p, d end
    end
  end
  return best, bd
end

function teamPlayers(ti)
  local r = {}
  for _, p in ipairs(M.players) do if p.t == ti then r[#r + 1] = p end end
  return r
end

-- ---------------- chutes e passes ----------------
function releaseBall(p)
  local b = M.ball
  b.owner = nil; b.last = p.t
  p.kick = 22; p.hold = 0
  b.x = p.x + p.fx * 8; b.y = p.y + p.fy * 8
end

function doShoot(p, power, aimx, aimy)
  local b = M.ball
  releaseBall(p)
  local tx, ty = atkX(p.t), FW / 2
  local dx, dy = norm(tx - p.x, ty - p.y)
  if aimx then
    dx = aimx * 0.7 + dx * 0.3; dy = aimy * 0.7 + dy * 0.3
    dx, dy = norm(dx, dy)
  end
  local err = (1.15 - power) * 0.10 + 0.015
  if p.t == 2 or not M.humanShot then err = err + (3 - OPT.diff) * 0.012 end
  local ang = math.atan2(dy, dx) + rnd(-err, err)
  local sp = 6.5 + power * 8
  b.vx, b.vy = math.cos(ang) * sp, math.sin(ang) * sp
  b.vz = 0.7 + power * 1.7 + rnd(0, 0.5)
  M.shots[p.t] = M.shots[p.t] + 1
  play("kick")
end

function pickMate(p, fxv, fyv)
  local best, bs = nil, -1e9
  for _, q in ipairs(M.players) do
    if q.t == p.t and q ~= p and not (q.gk and not p.gk and false) then
      local dx, dy, d = norm(q.x - p.x, q.y - p.y)
      if d > 20 then
        local dot = dx * fxv + dy * fyv
        local s = dot * 1.0 - d / 700
        if q.gk then s = s - 0.6 end
        if dot > 0.25 and s > bs then best, bs = q, s end
      end
    end
  end
  if not best then
    local bd = 1e9
    for _, q in ipairs(M.players) do
      if q.t == p.t and q ~= p and not q.gk then
        local d = dist(p.x, p.y, q.x, q.y)
        if d < bd then best, bd = q, d end
      end
    end
  end
  return best
end

function doPass(p, lob, fxv, fyv, human)
  local b = M.ball
  local q = pickMate(p, fxv or p.fx, fyv or p.fy)
  releaseBall(p)
  if not q then
    local sp = 6
    b.vx, b.vy = p.fx * sp, p.fy * sp
    play("kick"); return
  end
  local d = dist(p.x, p.y, q.x, q.y)
  local tx = q.x + q.vx * 8
  local ty = q.y + q.vy * 8
  local dx, dy = norm(tx - p.x, ty - p.y)
  if lob then
    local vz = clamp(d * 0.008 + 1.8, 2.0, 4.6)
    local T = 2 * vz / GRAV
    local sp = clamp(d / T, 2, 11)
    b.vx, b.vy, b.vz = dx * sp, dy * sp, vz
  else
    local sp = clamp(3.6 + d * 0.012, 4, 10)
    b.vx, b.vy, b.vz = dx * sp, dy * sp, 0
  end
  play("kick")
  if human and p.t == 1 and not q.gk then M.ctrl = q end
end

function attemptTackle(p, o)
  local b = M.ball
  p.tack = 40
  local chance = 0.42 + (TEAMS[M.team[p.t]].str - TEAMS[M.team[o.t]].str) * 0.04
  if p.t == 2 then chance = chance + (OPT.diff - 2) * 0.05 end
  if math.random() < chance then
    o.stun = 30; b.owner = p; b.last = p.t; p.hold = 0
    if p.t == 1 and not p.gk then M.ctrl = p end
    return true
  else
    p.stun = 22
    return false
  end
end

-- ---------------- IA ----------------
function diffMul(p)
  local m = 1.0
  if p.t == 2 then m = ({ 0.84, 0.94, 1.03 })[OPT.diff] or 0.94 end
  return m * (0.94 + 0.03 * TEAMS[M.team[p.t]].str)
end

function aiWithBall(p)
  local b = M.ball
  local dir = M.dir[p.t]
  local ax = atkX(p.t)
  p.hold = p.hold + 1
  local dg = dist(p.x, p.y, ax, FW / 2)
  if p.gk then
    if p.hold > 55 then
      local fx, fy = dir, rnd(-0.4, 0.4)
      doPass(p, math.random() < 0.4, fx, fy, false)
    end
    return p.x, p.y, 0
  end
  local range = M.futsal and 250 or 290
  if dg < range and p.hold > 8 and math.random() < 0.045 + 0.01 * TEAMS[M.team[p.t]].str then
    local ay = FW / 2 + rnd(-45, 45)
    M.humanShot = false
    doShoot(p, rnd(0.6, 1.0), nil, nil)
    return p.x, p.y, 0
  end
  -- adversario proximo?
  local nd, no = 1e9, nil
  for _, o in ipairs(M.players) do
    if o.t ~= p.t then
      local d = dist(p.x, p.y, o.x, o.y)
      if d < nd then nd, no = d, o end
    end
  end
  if nd < 22 and math.random() < 0.05 then
    doPass(p, math.random() < 0.25, p.fx, p.fy, false)
    return p.x, p.y, 0
  end
  local ty = FW / 2 + math.sin(M.hf / 45 + p.i) * 70
  if no and nd < 60 and (no.x - p.x) * dir > 0 then
    if no.y > p.y then ty = p.y - 70 else ty = p.y + 70 end
  end
  return ax, clamp(ty, 20, FW - 20), 1
end

function aiPlayer(p)
  local b = M.ball
  local ti = p.t
  local dir = M.dir[ti]
  local hx, hy = homePos(p)
  local own = ownX(ti)
  if p.gk then
    if b.owner == p then return aiWithBall(p) end
    local tx = own + dir * (10 + clamp(math.abs(b.x - own) * 0.1, 0, 28))
    local ty = FW / 2 + (b.y - FW / 2) * 0.42
    if not b.owner and math.abs(b.x - own) < 120 and b.z < 20 and
       math.abs(b.vx) < 3.5 and dist(p.x, p.y, b.x, b.y) < 80 then
      return b.x, b.y, 1
    end
    return tx, clamp(ty, 165, 235), 1
  end
  if b.owner == p then return aiWithBall(p) end
  local o = b.owner
  -- quem persegue?
  local ch = nearestField(ti, b.x, b.y, (ti == 1) and M.ctrl or nil)
  if o and o.t == ti then
    -- time com a bola: apoia
    local x = hx + dir * 90
    return clamp(x, 40, FL - 40), hy, 1
  end
  if ch == p then
    if o then
      local d = dist(p.x, p.y, o.x, o.y)
      if d < 14 and p.tack <= 0 and math.random() < 0.03 + 0.012 * TEAMS[M.team[ti]].str then
        attemptTackle(p, o)
      end
      return o.x + o.vx * 6, o.y + o.vy * 6, 1
    end
    return b.x + b.vx * 4, b.y + b.vy * 4, 1
  end
  -- segundo mais proximo tambem ajuda na bola livre
  if not o then
    local d1 = dist(p.x, p.y, b.x, b.y)
    if d1 < 130 then return b.x, b.y, 0.95 end
  end
  local x = hx * 0.55 + b.x * 0.45
  local y = hy * 0.7 + b.y * 0.3
  return x, y, 0.9
end

-- ---------------- movimento ----------------
function movePlayer(p, tx, ty, sf)
  local sp = 1.5 * sf * diffMul(p)
  if p.stun > 0 then sp = 0 end
  local dx, dy, d = norm(tx - p.x, ty - p.y)
  if d < 3 then dx, dy = 0, 0 end
  local lim = math.min(1, d / 14)
  local wvx, wvy = dx * sp * lim, dy * sp * lim
  if p.tack > 28 then wvx, wvy = p.fx * 2.5, p.fy * 2.5 end
  p.vx = p.vx + (wvx - p.vx) * 0.28
  p.vy = p.vy + (wvy - p.vy) * 0.28
  p.x = p.x + p.vx
  p.y = p.y + p.vy
  if math.abs(p.vx) + math.abs(p.vy) > 0.25 then
    p.moving = true
    p.anim = p.anim + 1
    local fx, fy = norm(p.vx, p.vy)
    if fx ~= 0 or fy ~= 0 then p.fx, p.fy = fx, fy end
  else
    p.moving = false
  end
  p.x = clamp(p.x, 4, FL - 4)
  p.y = clamp(p.y, 4, FW - 4)
end

function humanControl(p)
  local b = M.ball
  local dx, dy = dirInput()
  local sf = 1.0
  if held("r") then sf = 1.35 end
  local has = (b.owner == p)
  if p.stun > 0 or M.state ~= "play" then dx, dy = 0, 0 end
  if has then
    -- passe
    if hit("cross") then
      local fx, fy = p.fx, p.fy
      if dx ~= 0 or dy ~= 0 then fx, fy = norm(dx, dy) end
      doPass(p, false, fx, fy, true); M.charge = 0
    elseif hit("square") then
      local fx, fy = p.fx, p.fy
      if dx ~= 0 or dy ~= 0 then fx, fy = norm(dx, dy) end
      doPass(p, true, fx, fy, true); M.charge = 0
    end
    if b.owner == p then
      if held("circle") then
        M.charge = math.min(1, M.charge + 1 / 40)
      elseif M.charge > 0 then
        local fx, fy = p.fx, p.fy
        if dx ~= 0 or dy ~= 0 then fx, fy = norm(dx, dy) end
        M.humanShot = true
        doShoot(p, math.max(0.25, M.charge), fx, fy)
        M.charge = 0
      end
    end
  else
    M.charge = 0
    if hit("cross") then
      local n = nearestField(1, b.x, b.y, p)
      if n then M.ctrl = n end
    end
    if hit("circle") and p.tack <= 0 and p.stun <= 0 then p.tack = 36 end
    if p.tack > 28 and b.owner and b.owner.t ~= p.t then
      if dist(p.x, p.y, b.owner.x, b.owner.y) < 15 then
        attemptTackle(p, b.owner); p.tack = 26
      end
    end
  end
  return p.x + dx * 40, p.y + dy * 40, ((dx ~= 0 or dy ~= 0) and sf or 0)
end

-- ---------------- bola ----------------
function setPiece(ti, x, y)
  local b = M.ball
  local q = nearestField(ti, x, y)
  if not q then q = M.players[1] end
  b.x, b.y, b.z, b.vx, b.vy, b.vz = x, y, 0, 0, 0, 0
  q.x, q.y = x - M.dir[ti] * 4, y
  if y < 8 then q.y = y + 6 elseif y > FW - 8 then q.y = y - 6 end
  b.owner = q; b.last = ti; q.hold = 0; q.kick = 0
  for _, p in ipairs(M.players) do p.vx, p.vy = 0, 0 end
  M.freeze = 28
  if ti == 1 and not q.gk then M.ctrl = q end
end

function onGoal(ti)   -- ti marcou
  M.score[ti] = M.score[ti] + 1
  M.state = "goal"; M.st = 170; M.scorer = ti
  play("goal")
end

function updateBall()
  local b = M.ball
  if b.owner then
    local p = b.owner
    b.x = p.x + p.fx * 6; b.y = p.y + p.fy * 6 + 1
    b.z, b.vx, b.vy, b.vz = 0, p.vx, p.vy, 0
    return
  end
  b.x = b.x + b.vx; b.y = b.y + b.vy
  b.z = b.z + b.vz; b.vz = b.vz - GRAV
  if b.z <= 0 then
    b.z = 0
    if b.vz < -1.2 then b.vz = -b.vz * 0.5 else b.vz = 0 end
    b.vx = b.vx * 0.985; b.vy = b.vy * 0.985
  else
    b.vx = b.vx * 0.997; b.vy = b.vy * 0.997
  end
  if math.abs(b.vx) < 0.02 then b.vx = 0 end
  if math.abs(b.vy) < 0.02 then b.vy = 0 end
  if M.state ~= "play" then return end
  -- linhas
  if b.y < 0 or b.y > FW then
    if M.futsal then
      b.y = clamp(b.y, 0, FW); b.vy = -b.vy * 0.8
    else
      local ti = (b.last == 1) and 2 or 1
      setPiece(ti, clamp(b.x, 12, FL - 12), (b.y < 0) and 3 or FW - 3)
      return
    end
  end
  if b.x < 0 or b.x > FL then
    local left = (b.x < 0)
    if b.y > GOAL_Y1 and b.y < GOAL_Y2 and b.z < GOAL_H then
      local scorer = (M.dir[1] == (left and -1 or 1)) and 1 or 2
      onGoal(scorer)
      return
    end
    if M.futsal then
      b.x = clamp(b.x, 0, FL); b.vx = -b.vx * 0.75
      if b.y > GOAL_Y1 - 8 and b.y < GOAL_Y2 + 8 then play("post") end
    else
      local defT = left and ((M.dir[1] == 1) and 1 or 2) or ((M.dir[1] == 1) and 2 or 1)
      if b.last == defT then
        local attT = (defT == 1) and 2 or 1
        setPiece(attT, left and 6 or FL - 6, (b.y < FW / 2) and 6 or FW - 6)
      else
        -- tiro de meta
        local gk = nil
        for _, p in ipairs(M.players) do if p.t == defT and p.gk then gk = p end end
        if gk then
          b.x, b.y, b.z, b.vx, b.vy, b.vz = ownX(defT) + M.dir[defT] * 20, FW / 2, 0, 0, 0, 0
          gk.x, gk.y = b.x - M.dir[defT] * 4, b.y
          b.owner = gk; b.last = defT; gk.hold = 0; gk.kick = 0
        end
      end
    end
  end
end

function updatePickup()
  local b = M.ball
  if b.owner or M.state ~= "play" or M.freeze > 0 then return end
  local best, bd = nil, 1e9
  for _, p in ipairs(M.players) do
    local reach = p.gk and 15 or 9
    local hz = p.gk and 34 or 13
    if p.kick <= 0 and p.stun <= 0 and b.z <= hz then
      local d = dist(p.x, p.y, b.x, b.y)
      if d < reach and d < bd then best, bd = p, d end
    end
  end
  if not best then return end
  local sp = math.sqrt(b.vx * b.vx + b.vy * b.vy)
  if best.gk then
    local cc = ({ 0.40, 0.52, 0.64 })[OPT.diff] or 0.52
    if best.t == 1 then cc = 0.62 end
    cc = cc + TEAMS[M.team[best.t]].str * 0.02
    if sp > 7 and math.random() > cc then
      b.vx = -b.vx * 0.45 + rnd(-1.2, 1.2)
      b.vy = b.vy * 0.5 + rnd(-3, 3)
      b.vz = rnd(0.6, 1.8)
      best.kick = 25; b.last = best.t
      play("post")
      return
    end
  else
    if sp > 11 then
      b.vx = b.vx * 0.5; b.vy = b.vy * 0.5; best.kick = 12
      return
    end
  end
  b.owner = best; b.last = best.t; best.hold = 0
  b.vx, b.vy, b.vz = 0, 0, 0
  if best.t == 1 and not best.gk and not M.training then M.ctrl = best end
end

-- ---------------- atualizacao ----------------
function separatePlayers()
  local n = #M.players
  for i = 1, n - 1 do
    local a = M.players[i]
    for j = i + 1, n do
      local c = M.players[j]
      local dx, dy = c.x - a.x, c.y - a.y
      local d = math.sqrt(dx * dx + dy * dy)
      if d < 9 and d > 0.01 then
        local push = (9 - d) * 0.5
        local ux, uy = dx / d, dy / d
        a.x = a.x - ux * push; a.y = a.y - uy * push
        c.x = c.x + ux * push; c.y = c.y + uy * push
      end
    end
  end
end

function updateMatch()
  local b = M.ball
  if hit("start") and (M.state == "play" or M.state == "kick") then
    M.paused = true; M.pmenu = 1; return
  end
  for _, p in ipairs(M.players) do
    if p.kick > 0 then p.kick = p.kick - 1 end
    if p.stun > 0 then p.stun = p.stun - 1 end
    if p.tack > 0 then p.tack = p.tack - 1 end
  end
  if M.freeze > 0 then M.freeze = M.freeze - 1 end

  if M.state == "kick" then
    M.st = M.st - 1
    if M.st == 40 then play("whistle") end
    if M.st <= 0 then M.state = "play" end
    for _, p in ipairs(M.players) do
      local hx, hy = homePos(p)
      if p == M.ctrl then humanControl(p) end
    end
    updateBall()
    return
  elseif M.state == "goal" then
    M.st = M.st - 1
    updateBall()
    b.x = clamp(b.x, -30, FL + 30)
    for _, p in ipairs(M.players) do
      p.vx, p.vy = p.vx * 0.8, p.vy * 0.8
      p.moving = false
    end
    if M.st <= 0 then
      if M.training then startKickoff(1) else startKickoff((M.scorer == 1) and 2 or 1) end
    end
    return
  elseif M.state == "half" then
    M.st = M.st - 1
    if M.st <= 0 or hit("cross") then
      M.half = 2; M.hf = 0
      M.dir = { -1, 1 }
      startKickoff(2)
    end
    return
  elseif M.state == "full" then
    if hit("cross") or hit("start") then finishMatch() end
    return
  end

  -- jogo rolando
  if not M.training then
    M.hf = M.hf + 1
    if M.hf >= M.halfFrames then
      play("whistle")
      if M.half == 1 then M.state = "half"; M.st = 240
      else M.state = "full" end
      return
    end
  end

  if not M.ctrl or M.ctrl.gk then
    if M.training then
      for _, p in ipairs(M.players) do if p.t == 1 then M.ctrl = p end end
    else
      M.ctrl = nearestField(1, b.x, b.y)
    end
  end
  for _, p in ipairs(M.players) do
    local tx, ty, sf
    if p == M.ctrl then
      tx, ty, sf = humanControl(p)
    else
      tx, ty, sf = aiPlayer(p)
    end
    if M.freeze > 0 then sf = 0 end
    movePlayer(p, tx, ty, sf)
  end
  separatePlayers()
  updatePickup()
  updateBall()
end

function finishMatch()
  local cb = M.onEnd
  local s1, s2 = M.score[1], M.score[2]
  SCENE = "result"
  RES = { s1 = s1, s2 = s2, t1 = M.team[1], t2 = M.team[2], cb = cb, shots = { M.shots[1], M.shots[2] } }
end

-- ---------------- desenho ----------------
function drawMatch()
  drawBG(M.bg)
  local items = {}
  for _, p in ipairs(M.players) do items[#items + 1] = { y = p.y, p = p } end
  items[#items + 1] = { y = M.ball.y + 0.5, ball = true }
  table.sort(items, function(a, c) return a.y < c.y end)
  for _, it in ipairs(items) do
    if it.ball then
      local b = M.ball
      local sx, sy = toScreen(b.x, b.y)
      frame("shadow", sx, sy + 1)
      frame("ball", sx, sy - b.z * 0.55)
    else
      local p = it.p
      local sx, sy = toScreen(p.x, p.y)
      frame("shadow", sx, sy)
      frame((p.t == 1) and "ring_h" or "ring_a", sx, sy)
      local key = p.kit .. "_s"
      local oy = 0
      if p.moving then
        key = p.kit .. (((math.floor(p.anim / 7) % 2) == 0) and "_a" or "_b")
        oy = -((math.floor(p.anim / 7) % 2))
      end
      if p.stun > 0 then key = p.kit .. "_s"; oy = 1 end
      frame(key, sx, sy + oy)
      if p == M.ctrl and M.state ~= "goal" then frame("arrow", sx, sy - 5 + math.floor(M.hf / 8) % 2) end
    end
  end
  -- barra de forca
  if M.ctrl and M.charge > 0 then
    local sx, sy = toScreen(M.ctrl.x, M.ctrl.y)
    box(sx - 15, sy + 3, 30, 4, C.black)
    box(sx - 14, sy + 4, math.floor(28 * M.charge), 2, (M.charge > 0.85) and C.red or C.green)
  end
  drawHUD()
end

function drawHUD()
  box(150, 0, 180, 22, Color.new(0, 0, 0, 170))
  local t1, t2 = TEAMS[M.team[1]], TEAMS[M.team[2]]
  if M.training then
    ctxt(3, "TREINO  GOLS: " .. M.score[1], C.yellow)
  else
    txt(156, 7, t1.short, C.white)
    ctxt(7, M.score[1] .. " x " .. M.score[2], C.yellow)
    txt(330 - 8 - 24, 7, t2.short, C.white)
    local mins = math.floor(M.hf / M.halfFrames * 45) + ((M.half == 2) and 45 or 0)
    box(210, 22, 60, 12, Color.new(0, 0, 0, 150))
    ctxt(24, string.format("%02d'", math.min(mins, 90)), C.white)
  end
  if M.state == "kick" and M.st > 20 then ctxt(110, "PREPARAR...", C.white) end
  if M.state == "goal" then
    box(120, 95, 240, 50, C.dim)
    ctxt(103, "GOOOL!", C.yellow)
    local nm = TEAMS[M.team[M.scorer]].name
    ctxt(122, nm, C.white)
  end
  if M.state == "half" then
    box(120, 95, 240, 56, C.dim)
    ctxt(103, "FIM DO 1o TEMPO", C.yellow)
    ctxt(122, M.score[1] .. " x " .. M.score[2] .. "   (X continua)", C.white)
  end
  if M.state == "full" then
    box(110, 90, 260, 66, C.dim)
    ctxt(98, "FIM DE JOGO", C.yellow)
    ctxt(116, TEAMS[M.team[1]].name .. " " .. M.score[1] .. " x " .. M.score[2] .. " " .. TEAMS[M.team[2]].name, C.white)
    ctxt(134, "X para continuar", C.grey)
  end
end

function drawPause()
  box(150, 70, 180, 112, C.panel)
  ctxt(78, "PAUSA", C.yellow)
  local items = { "CONTINUAR", "REINICIAR PARTIDA", "SAIR PARA O MENU" }
  for i, s in ipairs(items) do
    ctxt(104 + (i - 1) * 20, s, (M.pmenu == i) and C.sel or C.white)
  end
end

function sceneMatch()
  if not M then SCENE = "menu"; return end
  if M.paused then
    drawMatch(); drawPause()
    if hit("up") then M.pmenu = math.max(1, M.pmenu - 1) end
    if hit("down") then M.pmenu = math.min(3, M.pmenu + 1) end
    if hit("start") or hit("triangle") then M.paused = false end
    if hit("cross") then
      if M.pmenu == 1 then M.paused = false
      elseif M.pmenu == 2 then
        local o = { t1 = M.team[1], t2 = M.team[2], futsal = M.futsal, training = M.training, onEnd = M.onEnd, trainKit = M.trainKit }
        newMatch(o)
      else
        M = nil; SCENE = "menu"; MENU.sel = 1
      end
    end
    return
  end
  updateMatch()
  if M then drawMatch() end
end

-- ============================================================
--  PENALTIS
-- ============================================================
PEN = nil
function newPens(o)
  PEN = {
    team = { o.t1, o.t2 }, kicks = { 0, 0 }, goals = { 0, 0 }, hist = { {}, {} },
    turn = 1, ph = "aim", aimY = 200, aimZ = 1, power = 0, pdir = 1, st = 0,
    gkY = 200, shotY = 200, shotZ = 6, res = "", onEnd = o.onEnd, bg = "bg_field", t = 0
  }
  SCENE = "pens"
end

function pensWinner()
  local k1, k2, g1, g2 = PEN.kicks[1], PEN.kicks[2], PEN.goals[1], PEN.goals[2]
  if k1 <= 5 and k2 <= 5 then
    if g1 > g2 + (5 - k2) then return 1 end
    if g2 > g1 + (5 - k1) then return 2 end
    if k1 == 5 and k2 == 5 and g1 ~= g2 then return (g1 > g2) and 1 or 2 end
    return nil
  end
  if k1 == k2 and g1 ~= g2 then return (g1 > g2) and 1 or 2 end
  return nil
end

function penResolve(shotY, shotZ, power, gkY, gkZ)
  if shotY < GOAL_Y1 + 4 or shotY > GOAL_Y2 - 4 then return "FORA!" end
  if power > 0.97 and shotZ > 15 then return "NA TRAVE... PRA FORA!" end
  local hitY = math.abs(gkY - shotY) < (18 + OPT.diff * 2)
  local hitZ = math.abs(gkZ - shotZ) < 18
  if hitY and hitZ and math.random() < 0.88 then return "DEFENDEU!" end
  if power < 0.3 and math.abs(gkY - shotY) < 40 then return "DEFENDEU!" end
  return "GOOOL!"
end

function penStartShot()
  local P = PEN
  P.ph = "shoot"; P.t = 0
  play("kick")
end

function penFinishKick(result)
  local P = PEN
  local ti = P.turn
  P.kicks[ti] = P.kicks[ti] + 1
  local gol = (result == "GOOOL!")
  if gol then P.goals[ti] = P.goals[ti] + 1; play("goal") end
  P.hist[ti][#P.hist[ti] + 1] = gol
  P.res = result
  P.ph = "result"; P.st = 100
end

function scenePens()
  local P = PEN
  if not P then SCENE = "menu"; return end
  drawBG(P.bg)
  local human = true
  local ti = P.turn
  local kicker = (ti == 1)       -- jogador humano chuta quando e o time 1
  local spotX = FL - 110
  local sx0, sy0 = toScreen(spotX, FW / 2)
  local gx, gy = toScreen(FL, P.gkY)
  local kitK = TEAMS[P.team[ti]].kit
  if TEAMS[P.team[ti]].kits then kitK = TEAMS[P.team[ti]].kits[1] end
  local kitG = "gk"
  local oti = (ti == 1) and 2 or 1

  if P.ph == "aim" then
    if kicker then
      if held("up") then P.aimY = math.max(125, P.aimY - 1.6) end
      if held("down") then P.aimY = math.min(275, P.aimY + 1.6) end
      if hit("left") then P.aimZ = 1 end
      if hit("right") then P.aimZ = 2 end
      if hit("cross") then P.ph = "power"; P.power = 0; P.pdir = 1 end
      P.gkY = FW / 2
    else
      if held("up") then P.aimY = math.max(150, P.aimY - 1.6) end
      if held("down") then P.aimY = math.min(250, P.aimY + 1.6) end
      if hit("left") then P.aimZ = 1 end
      if hit("right") then P.aimZ = 2 end
      P.gkY = FW / 2
      if hit("cross") then
        local sy = rnd(155, 245)
        if math.random() < 0.1 then sy = (math.random() < 0.5) and 135 or 265 end
        P.shotY = sy; P.shotZ = (math.random() < 0.5) and 6 or 22
        P.power = rnd(0.6, 0.95)
        P.gkDive = P.aimY; P.gkDiveZ = (P.aimZ == 2) and 22 or 6
        penStartShot()
      end
    end
  elseif P.ph == "power" then
    P.power = P.power + 0.024 * P.pdir
    if P.power >= 1 then P.power = 1; P.pdir = -1 end
    if P.power <= 0 then P.power = 0; P.pdir = 1 end
    if hit("cross") then
      local e = P.power - 0.85
      P.shotY = P.aimY + rnd(-1, 1) * math.abs(e) * 70 + rnd(-4, 4)
      P.shotZ = (P.aimZ == 2) and 22 or 6
      P.gkDive = rnd(158, 242)
      P.gkDiveZ = (math.random() < 0.5) and 6 or 22
      penStartShot()
    end
  elseif P.ph == "shoot" then
    P.t = P.t + 1
    local k = P.t / 38
    P.gkY = FW / 2 + ((P.gkDive or FW / 2) - FW / 2) * math.min(1, P.t / 14)
    if P.t >= 38 then
      local r = penResolve(P.shotY, P.shotZ, P.power, P.gkDive or 200, P.gkDiveZ or 6)
      penFinishKick(r)
    end
  elseif P.ph == "result" then
    P.st = P.st - 1
    if P.st <= 0 or hit("cross") then
      local w = pensWinner()
      if w then
        P.ph = "end"; P.st = 140
      else
        if P.turn == 1 then P.turn = 2 else P.turn = 1 end
        P.ph = "aim"; P.aimY = 200; P.aimZ = 1; P.gkY = FW / 2
      end
    end
  elseif P.ph == "end" then
    P.st = P.st - 1
    if P.st <= 0 or hit("cross") then
      local w = pensWinner()
      local cb = P.onEnd
      PEN = nil
      if cb then cb(w) else SCENE = "menu" end
      return
    end
  end

  -- desenho
  local gsx, gsy = toScreen(FL - 6, P.gkY)
  frame("shadow", gsx, gsy); frame("ring_a", gsx, gsy)
  if (P.ph == "shoot" or P.ph == "result") and math.abs((P.gkDive or 200) - FW / 2) > 20 then
    frame("gk_a", gsx, gsy - ((P.gkDiveZ or 6) > 15 and 8 or 0))
  else
    frame("gk_s", gsx, gsy)
  end
  frame("shadow", sx0 - 14, sy0); frame("ring_h", sx0 - 14, sy0)
  frame(kitK .. ((P.ph == "shoot") and "_a" or "_s"), sx0 - 14, sy0)
  -- bola
  local bx, by, bz = sx0, sy0, 0
  if P.ph == "shoot" or P.ph == "result" then
    local k = math.min(1, (P.ph == "result") and 1 or P.t / 38)
    local ex, ey = toScreen(FL + 4, P.shotY)
    bx = sx0 + (ex - sx0) * k
    by = sy0 + (ey - sy0) * k
    bz = P.shotZ * math.sin(k * math.pi * 0.5) * 0.6
  end
  frame("shadow", bx, by + 1); frame("ball", bx, by - bz)
  -- mira
  if P.ph == "aim" or P.ph == "power" then
    local mx, my
    if kicker then mx, my = toScreen(FL + 2, P.aimY) else mx, my = toScreen(FL - 6, P.aimY) end
    local hz = (P.aimZ == 2) and 14 or 3
    screen:fillRect(mx - 4, my - hz - 4, 9, 2, C.yellow)
    screen:fillRect(mx - 1, my - hz - 7, 2, 9, C.yellow)
  end
  -- HUD
  box(110, 0, 260, 38, Color.new(0, 0, 0, 170))
  ctxt(4, TEAMS[P.team[1]].short .. "  " .. P.goals[1] .. " x " .. P.goals[2] .. "  " .. TEAMS[P.team[2]].short, C.yellow)
  for t = 1, 2 do
    local s = ""
    for i = 1, math.max(5, #P.hist[t]) do
      local h = P.hist[t][i]
      if h == nil then s = s .. ". " elseif h then s = s .. "O " else s = s .. "X " end
    end
    txt(120, 14 + (t - 1) * 11, string.sub(TEAMS[P.team[t]].short, 1, 3) .. " " .. s, C.white)
  end
  if P.ph == "aim" then
    if kicker then ctxt(228, "CIMA/BAIXO mira, ESQ/DIR altura, X confirma", C.white)
    else ctxt(228, "DEFESA: CIMA/BAIXO e ESQ/DIR escolhem o canto, X pula", C.white) end
  elseif P.ph == "power" then
    box(170, 236, 140, 14, C.black)
    box(172, 238, math.floor(136 * P.power), 10, (P.power > 0.97) and C.red or ((P.power > 0.7) and C.green or C.orange))
    ctxt(254, "X para chutar (ideal: perto do fim)", C.white)
  elseif P.ph == "result" then
    box(110, 100, 260, 36, C.dim)
    ctxt(112, P.res, C.yellow)
  elseif P.ph == "end" then
    local w = pensWinner()
    box(100, 96, 280, 50, C.dim)
    ctxt(104, "VENCEDOR NOS PENALTIS", C.yellow)
    ctxt(124, TEAMS[P.team[w]].name, C.white)
  end
end

-- ============================================================
--  CAMPEONATO / COPA
-- ============================================================
function simGoals(mu)
  mu = math.max(0.3, mu)
  local g = 0
  for i = 1, 8 do if math.random() < mu / 8 then g = g + 1 end end
  return g
end
function simMatch(a, b)
  local sa, sb = TEAMS[a].str, TEAMS[b].str
  return simGoals(1.3 + (sa - sb) * 0.28), simGoals(1.3 + (sb - sa) * 0.28)
end

LG = nil
function newLeague(user)
  LG = { user = user, round = 1, table = {}, rounds = {}, done = false }
  local arr = {}
  for i = 1, NCLUBS do arr[i] = i; LG.table[i] = { p = 0, w = 0, d = 0, l = 0, gf = 0, ga = 0, pts = 0 } end
  for r = 1, NCLUBS - 1 do
    local pairs_ = {}
    for i = 1, NCLUBS / 2 do pairs_[#pairs_ + 1] = { arr[i], arr[NCLUBS + 1 - i] } end
    LG.rounds[r] = pairs_
    local na = { arr[1], arr[NCLUBS] }
    for i = 2, NCLUBS - 1 do na[#na + 1] = arr[i] end
    arr = na
  end
  SCENE = "league"
end

function recordResult(tb, a, b, ga, gb)
  local A, B = tb[a], tb[b]
  A.p = A.p + 1; B.p = B.p + 1
  A.gf = A.gf + ga; A.ga = A.ga + gb; B.gf = B.gf + gb; B.ga = B.ga + ga
  if ga > gb then A.w = A.w + 1; A.pts = A.pts + 3; B.l = B.l + 1
  elseif gb > ga then B.w = B.w + 1; B.pts = B.pts + 3; A.l = A.l + 1
  else A.d = A.d + 1; B.d = B.d + 1; A.pts = A.pts + 1; B.pts = B.pts + 1 end
end

function sortedLeague()
  local ids = {}
  for i = 1, NCLUBS do ids[i] = i end
  table.sort(ids, function(a, b)
    local A, B = LG.table[a], LG.table[b]
    if A.pts ~= B.pts then return A.pts > B.pts end
    local da, db = A.gf - A.ga, B.gf - B.ga
    if da ~= db then return da > db end
    if A.gf ~= B.gf then return A.gf > B.gf end
    return a < b
  end)
  return ids
end

function leagueNextFixture()
  for _, pr in ipairs(LG.rounds[LG.round]) do
    if pr[1] == LG.user or pr[2] == LG.user then return pr end
  end
end

function leaguePlay()
  local pr = leagueNextFixture()
  local home = (pr[1] == LG.user)
  local a, b = pr[1], pr[2]
  newMatch({ t1 = LG.user, t2 = home and b or a, onEnd = function(s1, s2)
    local ga, gb
    if home then ga, gb = s1, s2 else ga, gb = s2, s1 end
    for _, q in ipairs(LG.rounds[LG.round]) do
      if q == pr then recordResult(LG.table, q[1], q[2], ga, gb)
      else
        local x, y = simMatch(q[1], q[2]); recordResult(LG.table, q[1], q[2], x, y)
      end
    end
    LG.round = LG.round + 1
    if LG.round > NCLUBS - 1 then LG.done = true end
    SCENE = "league"
  end })
  SCENE = "match"
end

function drawTable(ids, y0, hl)
  txt(30, y0, "#  TIME", C.grey)
  txt(250, y0, " J  V  E  D  GP  PTS", C.grey)
  for pos, id in ipairs(ids) do
    local r = LG.table[id]
    local y = y0 + 4 + pos * 16
    if id == hl then box(24, y - 2, 432, 14, Color.new(255, 200, 40, 70)) end
    txt(30, y, pos .. "  " .. TEAMS[id].name, (id == hl) and C.yellow or C.white)
    txt(250, y, string.format("%2d %2d %2d %2d %3d %4d", r.p, r.w, r.d, r.l, r.gf - r.ga, r.pts), C.white)
  end
end

function sceneLeague()
  drawBG("bg_field"); box(0, 0, SW, SH, Color.new(0, 0, 0, 170))
  ctxt(8, "CAMPEONATO", C.yellow)
  drawTable(sortedLeague(), 28, LG.user)
  if LG.done then
    local ids = sortedLeague()
    local pos = 1
    for i, id in ipairs(ids) do if id == LG.user then pos = i end end
    box(90, 205, 300, 50, C.panel)
    if pos == 1 then ctxt(212, "VOCE E O CAMPEAO!", C.yellow) else ctxt(212, "Terminou em " .. pos .. "o lugar", C.yellow) end
    ctxt(234, "X para voltar ao menu", C.white)
    if hit("cross") then LG = nil; SCENE = "menu" end
  else
    local pr = leagueNextFixture()
    local o = (pr[1] == LG.user) and pr[2] or pr[1]
    ctxt(214, "RODADA " .. LG.round .. "/" .. (NCLUBS - 1) .. ":  " .. TEAMS[LG.user].short .. " x " .. TEAMS[o].short, C.white)
    ctxt(234, "X jogar     TRIANGULO sair", C.grey)
    if hit("cross") then leaguePlay() end
    if hit("triangle") then LG = nil; SCENE = "menu" end
  end
end

-- copa
CP = nil
CUPNAMES = { "QUARTAS DE FINAL", "SEMIFINAL", "FINAL" }
function newCup(user)
  local ids = {}
  for i = 1, NCLUBS do if i ~= user then ids[#ids + 1] = i end end
  for i = #ids, 2, -1 do local j = math.random(1, i); ids[i], ids[j] = ids[j], ids[i] end
  table.insert(ids, 1, user)
  CP = { user = user, round = 1, slots = ids, alive = true, champion = false }
  SCENE = "cup"
end

function cupAdvance(userWon)
  local nxt = {}
  local sl = CP.slots
  for i = 1, #sl, 2 do
    local a, b = sl[i], sl[i + 1]
    local w
    if a == CP.user or b == CP.user then
      w = userWon and CP.user or ((a == CP.user) and b or a)
    else
      local x, y = simMatch(a, b)
      if x == y then
        if math.random() < 0.5 + (TEAMS[a].str - TEAMS[b].str) * 0.06 then x = x + 1 else y = y + 1 end
      end
      w = (x > y) and a or b
    end
    nxt[#nxt + 1] = w
  end
  CP.slots = nxt
  if not userWon then CP.alive = false
  elseif #nxt == 1 then CP.champion = true
  else CP.round = CP.round + 1 end
  SCENE = "cup"
end

function cupPlay()
  local sl = CP.slots
  local opp
  for i = 1, #sl, 2 do
    if sl[i] == CP.user then opp = sl[i + 1] elseif sl[i + 1] == CP.user then opp = sl[i] end
  end
  newMatch({ t1 = CP.user, t2 = opp, onEnd = function(s1, s2)
    if s1 ~= s2 then cupAdvance(s1 > s2)
    else
      newPens({ t1 = CP.user, t2 = opp, onEnd = function(w) cupAdvance(w == 1) end })
    end
  end })
  SCENE = "match"
end

function sceneCup()
  drawBG("bg_field"); box(0, 0, SW, SH, Color.new(0, 0, 0, 170))
  ctxt(10, "COPA", C.yellow)
  local sl = CP.slots
  if CP.champion then
    ctxt(110, "VOCE E O CAMPEAO DA COPA!", C.yellow)
    ctxt(140, TEAMS[CP.user].name, C.white)
    ctxt(220, "X para voltar ao menu", C.grey)
    if hit("cross") then CP = nil; SCENE = "menu" end
    return
  end
  if not CP.alive then
    ctxt(110, "VOCE FOI ELIMINADO", C.red)
    ctxt(220, "X para voltar ao menu", C.grey)
    if hit("cross") then CP = nil; SCENE = "menu" end
    return
  end
  ctxt(34, CUPNAMES[CP.round] or "FINAL", C.white)
  for i = 1, #sl, 2 do
    local y = 64 + ((i - 1) / 2) * 32
    local a, b = sl[i], sl[i + 1]
    local mine = (a == CP.user or b == CP.user)
    local col = mine and C.yellow or C.white
    ctxt(y, TEAMS[a].name .. "  x  " .. TEAMS[b].name, col)
  end
  ctxt(220, "X jogar     TRIANGULO sair", C.grey)
  if hit("cross") then cupPlay() end
  if hit("triangle") then CP = nil; SCENE = "menu" end
end

-- ============================================================
--  MENUS
-- ============================================================
MENU = { sel = 1, items = { "AMISTOSO", "CAMPEONATO", "COPA", "PENALTIS", "FUTSAL", "TREINO", "OPCOES", "SAIR" } }
TS = { idx = 1, title = "", cb = nil, avoid = nil, back = "menu" }
SCN = {}

function pickTeam(title, cb, avoid, onlyClubs)
  TS.title = title; TS.cb = cb; TS.avoid = avoid; TS.only = onlyClubs
  TS.idx = avoid and ((avoid == 1) and 2 or 1) or 1
  SCENE = "teamsel"
end

function SCN.title()
  drawBG("bg_field"); box(0, 0, SW, SH, Color.new(0, 0, 0, 90))
  if IMG.title then screen:blit(10, 20, IMG.title) else ctxt(60, "SUPER PIXEL SOCCER PSP", C.yellow) end
  local t = math.floor(TICK / 8)
  local kits = { "neymar", "messi", "pele", "cr7", "mara" }
  for i, k in ipairs(kits) do
    local f = ((t + i) % 2 == 0) and "_a" or "_b"
    frame(k .. f, 100 + i * 60, 215)
  end
  if math.floor(TICK / 30) % 2 == 0 then ctxt(148, "PRESSIONE START", C.white) end
  ctxt(250, "Jogo de fas - nao oficial", C.grey)
  if hit("start") or hit("cross") then SCENE = "menu" end
end

function SCN.menu()
  drawBG("bg_field"); box(0, 0, SW, SH, Color.new(0, 0, 0, 140))
  ctxt(14, "MENU PRINCIPAL", C.yellow)
  for i, s in ipairs(MENU.items) do
    local y = 40 + (i - 1) * 24
    if MENU.sel == i then box(150, y - 4, 180, 20, C.sel) end
    ctxt(y, s, (MENU.sel == i) and C.black or C.white)
  end
  if hit("up") then MENU.sel = (MENU.sel > 1) and MENU.sel - 1 or #MENU.items end
  if hit("down") then MENU.sel = (MENU.sel < #MENU.items) and MENU.sel + 1 or 1 end
  if hit("cross") then
    local s = MENU.items[MENU.sel]
    if s == "AMISTOSO" or s == "FUTSAL" then
      local fut = (s == "FUTSAL")
      pickTeam("SEU TIME", function(a)
        pickTeam("ADVERSARIO", function(b)
          newMatch({ t1 = a, t2 = b, futsal = fut, onEnd = function() SCENE = "menu" end }); SCENE = "match"
        end, a)
      end)
    elseif s == "CAMPEONATO" then
      pickTeam("SEU TIME", function(a) newLeague(a) end, nil, true)
    elseif s == "COPA" then
      pickTeam("SEU TIME", function(a) newCup(a) end, nil, true)
    elseif s == "PENALTIS" then
      pickTeam("SEU TIME", function(a)
        pickTeam("ADVERSARIO", function(b)
          newPens({ t1 = a, t2 = b, onEnd = function() SCENE = "menu" end })
        end, a)
      end)
    elseif s == "TREINO" then
      TR = { idx = 1 }; SCENE = "trainsel"
    elseif s == "OPCOES" then
      OP = { sel = 1 }; SCENE = "opts"
    elseif s == "SAIR" then
      pcall(function() System.Quit() end)
    end
  end
end

function SCN.teamsel()
  drawBG("bg_field"); box(0, 0, SW, SH, Color.new(0, 0, 0, 150))
  ctxt(8, TS.title, C.yellow)
  local n = TS.only and NCLUBS or #TEAMS
  for i = 1, n do
    local y = 30 + (i - 1) * 22
    local dis = (TS.avoid == i)
    if TS.idx == i then box(20, y - 3, 190, 19, C.sel) end
    txt(28, y + 1, TEAMS[i].name, (TS.idx == i) and C.black or (dis and C.grey or C.white))
  end
  local tm = TEAMS[TS.idx]
  box(230, 30, 230, 150, C.panel)
  ctxt(38, tm.name, C.yellow, 345)
  local kits = tm.kits or { tm.kit, tm.kit, tm.kit }
  for i, k in ipairs(kits) do
    local f = (math.floor(TICK / 10) % 2 == 0) and "_a" or "_b"
    frame(k .. f, 230 + 230 / (#kits + 1) * i, 110)
  end
  local stars = ""
  for i = 1, 5 do stars = stars .. ((i <= tm.str) and "*" or "-") end
  ctxt(130, "FORCA " .. stars, C.white, 345)
  ctxt(150, tm.short, tm.col, 345)
  ctxt(240, "X escolher     TRIANGULO voltar", C.grey)
  if hit("up") then TS.idx = (TS.idx > 1) and TS.idx - 1 or n end
  if hit("down") then TS.idx = (TS.idx < n) and TS.idx + 1 or 1 end
  if hit("triangle") then SCENE = "menu" end
  if hit("cross") and TS.avoid ~= TS.idx then
    local cb = TS.cb
    cb(TS.idx)
  end
end

STARS = { { "neymar", "NEYMAR" }, { "messi", "MESSI" }, { "pele", "PELE" }, { "cr7", "CR7" }, { "mara", "CRAQUE" } }
function SCN.trainsel()
  drawBG("bg_field"); box(0, 0, SW, SH, Color.new(0, 0, 0, 150))
  ctxt(10, "TREINO - ESCOLHA O CRAQUE", C.yellow)
  for i, s in ipairs(STARS) do
    local x = 40 + (i - 1) * 90
    if TR.idx == i then box(x - 28, 80, 60, 90, C.sel) end
    local f = (TR.idx == i and math.floor(TICK / 8) % 2 == 0) and "_a" or "_s"
    frame(s[1] .. f, x, 140)
    ctxt(150, "", C.white)
    txt(x - #s[2] * 4, 152, s[2], (TR.idx == i) and C.black or C.white)
  end
  ctxt(200, "Faca gols contra goleiro e 2 zagueiros", C.white)
  ctxt(240, "X jogar     TRIANGULO voltar", C.grey)
  if hit("left") then TR.idx = (TR.idx > 1) and TR.idx - 1 or #STARS end
  if hit("right") then TR.idx = (TR.idx < #STARS) and TR.idx + 1 or 1 end
  if hit("triangle") then SCENE = "menu" end
  if hit("cross") then
    newMatch({ t1 = 9, t2 = 2, training = true, trainKit = STARS[TR.idx][1] })
    SCENE = "match"
  end
end

function SCN.opts()
  drawBG("bg_field"); box(0, 0, SW, SH, Color.new(0, 0, 0, 150))
  ctxt(14, "OPCOES", C.yellow)
  local dn = { "FACIL", "NORMAL", "DIFICIL" }
  local hn = { "CURTO (45s)", "MEDIO (90s)", "LONGO (150s)" }
  local items = {
    "DIFICULDADE: " .. dn[OPT.diff],
    "TEMPO POR TEMPO: " .. hn[OPT.half],
    "SOM: " .. (OPT.sound and "LIGADO" or "DESLIGADO"),
    "VOLTAR"
  }
  for i, s in ipairs(items) do
    local y = 60 + (i - 1) * 30
    if OP.sel == i then box(100, y - 5, 280, 22, C.sel) end
    ctxt(y, s, (OP.sel == i) and C.black or C.white)
  end
  ctxt(200, "Controles: X passe | O chute (segure) | [] lob", C.grey)
  ctxt(214, "R correr | O sem bola = carrinho | START pausa", C.grey)
  if hit("up") then OP.sel = (OP.sel > 1) and OP.sel - 1 or 4 end
  if hit("down") then OP.sel = (OP.sel < 4) and OP.sel + 1 or 1 end
  local d = 0
  if hit("left") then d = -1 end
  if hit("right") or hit("cross") then d = 1 end
  if d ~= 0 then
    if OP.sel == 1 then OPT.diff = clamp(OPT.diff + d, 1, 3)
    elseif OP.sel == 2 then OPT.half = clamp(OPT.half + d, 1, 3)
    elseif OP.sel == 3 then OPT.sound = not OPT.sound
    elseif OP.sel == 4 and hit("cross") then saveOpt(); SCENE = "menu" end
  end
  if hit("triangle") then saveOpt(); SCENE = "menu" end
end

function SCN.result()
  drawBG("bg_field"); box(0, 0, SW, SH, Color.new(0, 0, 0, 160))
  ctxt(30, "RESULTADO FINAL", C.yellow)
  ctxt(80, TEAMS[RES.t1].name, C.white)
  ctxt(110, RES.s1 .. "  x  " .. RES.s2, C.yellow)
  ctxt(140, TEAMS[RES.t2].name, C.white)
  ctxt(180, "Chutes: " .. RES.shots[1] .. " x " .. RES.shots[2], C.grey)
  ctxt(230, "X continuar", C.white)
  if hit("cross") then
    local cb = RES.cb
    M = nil
    if cb then cb(RES.s1, RES.s2) else SCENE = "menu" end
  end
end

SCN.match = sceneMatch
SCN.pens = scenePens
SCN.league = sceneLeague
SCN.cup = sceneCup

-- ============================================================
--  INICIO
-- ============================================================
function boot()
  pcall(function() math.randomseed(os.time()) end)
  screen:clear(C.black)
  screen:print(180, 130, "Carregando...", C.white)
  flip()
  loadImg("atlas", "gfx/atlas.png")
  loadImg("bg_field", "gfx/bg_field.png")
  loadImg("bg_court", "gfx/bg_court.png")
  loadImg("title", "gfx/title.png")
  loadSnd("kick", "sfx/kick.wav")
  loadSnd("whistle", "sfx/whistle.wav")
  loadSnd("goal", "sfx/goal.wav")
  loadSnd("post", "sfx/post.wav")
  loadOpt()
  SCENE = "title"
  TICK = 0
end

function mainLoop()
  while true do
    TICK = TICK + 1
    readPad()
    local f = SCN[SCENE]
    if f then f() else SCENE = "menu" end
    flip()
  end
end

if not NO_AUTORUN then
  boot()
  mainLoop()
end
