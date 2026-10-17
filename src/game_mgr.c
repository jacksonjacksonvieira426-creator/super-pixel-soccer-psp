/* Super Pixel Soccer PSP - modo manager (carreira de treinador) */
#include "game.h"

Mgr MGR;

#define MGR_MAGIC 0x4D475231
#define MAXSQ 26
static const char *POSN[4] = { "GOL", "ZAG", "MEI", "ATA" };
static const char *TRAINN[5] = { "FISICO", "ATAQUE", "DEFESA", "TECNICO", "DESCANSO" };
const char *mgrPosName(int pos) { return POSN[clampi(pos, 0, 3)]; }

static int rn(int a, int b) { return a + rand() % (b - a + 1); }
static int priceOf(const MPl *p) {
  int o = p->ovr - 40; if (o < 0) o = 0;
  int pr = o * o * 4;
  if (p->age < 24) pr = pr * 13 / 10; else if (p->age > 30) pr = pr * 7 / 10;
  return pr < 50 ? 50 : pr;
}
static void genPlayer(MPl *p, int pos, int base) {
  p->name = rand() % 64; p->pos = pos;
  p->ovr = clampi(base + rn(-8, 8), 40, 92);
  p->age = rn(18, 34); p->goals = p->games = 0; p->fit = 100;
  p->price = priceOf(p);
}
static int avgXI(void) {
  int s = 0, n = 0;
  for (int i = 0; i < 11; i++) if (MGR.xi[i] >= 0 && MGR.xi[i] < MGR.nsq) { s += MGR.sq[MGR.xi[i]].ovr; n++; }
  return n ? s / n : 50;
}
static int totalOvr(void) { int s = 0; for (int i = 0; i < MGR.nsq; i++) s += MGR.sq[i].ovr; return s; }

/* preenche escalacao: mantem escolhas validas e completa pelo melhor de cada funcao */
static void fillXI(int reset) {
  int taken[MAXSQ]; memset(taken, 0, sizeof taken);
  if (reset) for (int i = 0; i < 11; i++) MGR.xi[i] = -1;
  for (int i = 0; i < 11; i++) {
    int x = MGR.xi[i];
    if (x < 0 || x >= MGR.nsq || taken[x]) MGR.xi[i] = -1; else taken[x] = 1;
  }
  for (int i = 0; i < 11; i++) {
    if (MGR.xi[i] >= 0) continue;
    int role = formRole(2, MGR.form, i), best = -1;
    for (int pass = 0; pass < 2 && best < 0; pass++)
      for (int k = 0; k < MGR.nsq; k++) {
        if (taken[k]) continue;
        if (pass == 0 && MGR.sq[k].pos != role) continue;
        if (best < 0 || MGR.sq[k].ovr > MGR.sq[best].ovr) best = k;
      }
    if (best >= 0) { MGR.xi[i] = best; taken[best] = 1; }
  }
}

static void genMarket(void) {
  int avg = avgXI();
  MGR.nmarket = 8;
  for (int i = 0; i < 8; i++) genPlayer(&MGR.market[i], (i < 1) ? 0 : ((i < 3) ? 1 : ((i < 6) ? 2 : 3)), avg + rn(-6, 10));
}

void mgrNew(int team, int coach) {
  memset(&MGR, 0, sizeof(MGR));
  MGR.magic = MGR_MAGIC; MGR.active = 1; MGR.team = team; MGR.coach = coach; MGR.season = 1;
  int str = TEAMS[team].str;
  MGR.money = 3000 + 1500 * str;
  static const int cnt[4] = { 2, 6, 6, 4 };
  for (int pos = 0; pos < 4; pos++)
    for (int k = 0; k < cnt[pos]; k++) genPlayer(&MGR.sq[MGR.nsq++], pos, 45 + 6 * str);
  MGR.tac = UserTac; MGR.form = 0;
  fillXI(1);
  genMarket();
  MGR.boardGoal = clampi(7 - (avgXI() - 55) / 4, 1, 10);
  leagueStart(team);
  MGR.sub = 10;           /* escolher treinador */
  SCENE = SC_MGR;
}

/* ---- ganchos usados pela partida ---- */
int mgrAssign(int slot, int role, int *nameIdx, float *pace, float *shot, float *defn, int *roster) {
  (void)role;
  if (slot == 0) { fillXI(0); int a = avgXI(); TEAMS[MGR.team].str = clampi((a - 45 + 3) / 6, 1, 5); }
  if (slot < 0 || slot > 10) return 0;
  int idx = MGR.xi[slot];
  if (idx < 0 || idx >= MGR.nsq) return 0;
  const MPl *p = &MGR.sq[idx];
  float eff = p->ovr * (0.85f + 0.15f * p->fit / 100.0f);
  *nameIdx = p->name;
  *pace = clampf(0.72f + eff * 0.0048f, 0.85f, 1.15f);
  *shot = clampf(0.70f + eff * 0.0050f, 0.85f, 1.15f);
  *defn = clampf(0.80f + eff * 0.0035f, 0.85f, 1.2f);
  *roster = idx;
  return 1;
}
void mgrCredit(int roster) { if (roster >= 0 && roster < MGR.nsq) MGR.sq[roster].goals++; }

void mgrAfterMatch(int s1, int s2) {
  int played[MAXSQ]; memset(played, 0, sizeof played);
  for (int i = 0; i < 11; i++) if (MGR.xi[i] >= 0) played[MGR.xi[i]] = 1;
  for (int k = 0; k < MGR.nsq; k++) {
    MPl *p = &MGR.sq[k];
    if (played[k]) { p->games++; p->fit = clampi(p->fit - 14, 30, 100); }
    else p->fit = clampi(p->fit + 12, 0, 100);
    if (MGR.train == 4) p->fit = clampi(p->fit + 12, 0, 100);
    else if (MGR.train == 0) p->fit = clampi(p->fit + 4, 0, 100);
    /* progresso pelo foco do treino */
    int match = (MGR.train == 1 && p->pos == 3) || (MGR.train == 2 && (p->pos == 1 || p->pos == 0)) ||
                (MGR.train == 3 && p->pos == 2) || (MGR.train == 0);
    float pr = match ? 0.11f : 0.03f;
    if (p->age < 24) pr *= 1.6f; else if (p->age > 30) pr *= 0.4f;
    if (MGR.train == 4) pr *= 0.3f;
    if (prob() < pr && p->ovr < 92) { p->ovr++; p->price = priceOf(p); }
  }
  int avg = avgXI();
  MGR.lastIncome = 500 + 12 * avg + ((s1 > s2) ? 300 : ((s1 == s2) ? 100 : 0));
  MGR.lastWages = totalOvr() / 3;
  MGR.money += MGR.lastIncome - MGR.lastWages;
  genMarket();
  mgrSave();
}

/* ---- salvar / carregar ---- */
int mgrSaved(void) {
  FILE *f = fopen("manager.dat", "rb");
  if (!f) return 0;
  int m = 0; int ok = (fread(&m, sizeof m, 1, f) == 1 && m == MGR_MAGIC);
  fclose(f); return ok;
}
void mgrSave(void) {
  FILE *f = fopen("manager.dat", "wb");
  if (!f) return;
  fwrite(&MGR, sizeof(MGR), 1, f);
  leagueWrite(f);
  fclose(f);
}
int mgrLoad(void) {
  FILE *f = fopen("manager.dat", "rb");
  if (!f) return 0;
  Mgr tmp;
  int ok = (fread(&tmp, sizeof(tmp), 1, f) == 1 && tmp.magic == MGR_MAGIC);
  if (ok) { MGR = tmp; ok = leagueRead(f); MGR.active = ok; MGR.sub = 0; }
  fclose(f);
  return ok;
}
void mgrClear(void) { remove("manager.dat"); }

/* ---------------- interface ---------------- */
int hubSelDebug = -1;
static int hubSel = 0, listTop = 0, lsSlot = 0, lsStage = 0, lsCand = 0, tab = 0, mSel = 0, tacSel2 = 0, trSel = 0;
static char msgLine[64];

static void moneyStr(char *out, int n, int k) {
  if (k >= 1000 || k <= -1000) snprintf(out, n, "R$ %d.%d mi", k / 1000, abs(k % 1000) / 100);
  else snprintf(out, n, "R$ %d mil", k);
}
static void bg(void) { drawBG(T_MENU); }

static const char *tip(void) {
  int s[4] = { 0, 0, 0, 0 }, n[4] = { 0, 0, 0, 0 };
  for (int i = 0; i < 11; i++) if (MGR.xi[i] >= 0) { MPl *p = &MGR.sq[MGR.xi[i]]; s[p->pos] += p->ovr; n[p->pos]++; }
  int a[4]; for (int k = 0; k < 4; k++) a[k] = n[k] ? s[k] / n[k] : 0;
  int tired = 0; for (int i = 0; i < 11; i++) if (MGR.xi[i] >= 0 && MGR.sq[MGR.xi[i]].fit < 60) tired++;
  if (tired >= 3) return "DICA: muitos titulares cansados. Descanse ou rode o time.";
  if (a[3] && a[3] + 5 < a[2]) return "DICA: seu ataque esta fraco. Contrate um atacante.";
  if (a[1] && a[1] + 5 < a[2]) return "DICA: a defesa esta fragil. Procure zagueiros.";
  if (MGR.money > 8000) return "DICA: caixa alto. Invista em jogadores no mercado.";
  if (MGR.money < 0) return "DICA: o clube esta no vermelho. Venda jogadores.";
  return "DICA: mude a tatica conforme o adversario.";
}

static void drawHubHeader(void) {
  char b[64];
  if (T_COACH >= 0) plat_blit(T_COACH, MGR.coach * 64, 0, 64, 64, 14, 30, 255, 255, 255, 255);
  txt(14, 98, TEAMS[MGR.team].name, C_YELLOW);
  snprintf(b, sizeof b, "TEMPORADA %d", MGR.season); txt(14, 112, b, C_WHITE);
  moneyStr(b, sizeof b, MGR.money); txt(14, 126, b, (MGR.money < 0) ? C_RED : C_GREEN);
  snprintf(b, sizeof b, "POSICAO: %do", leagueUserPos()); txt(14, 140, b, C_WHITE);
  snprintf(b, sizeof b, "META: top %d", MGR.boardGoal); txt(14, 154, b, C_GREY);
  snprintf(b, sizeof b, "MEDIA XI: %d", avgXI()); txt(14, 168, b, C_GREY);
}

static void endSeason(void) {
  static const int prize[12] = { 5000, 3000, 2000, 1200, 800, 500, 300, 200, 100, 50, 0, 0 };
  int pos = leagueUserPos();
  MGR.lastPrize = prize[clampi(pos - 1, 0, 11)];
  MGR.money += MGR.lastPrize;
  if (pos == 1) MGR.trophies[0]++;
  if (pos <= 3) MGR.trophies[1]++;
  int best = 0; for (int k = 0; k < MGR.nsq; k++) if (MGR.sq[k].goals > best) best = MGR.sq[k].goals;
  MGR.scorerBest = best;
  if (best >= 10) MGR.trophies[2]++;
  if (leagueGoalDiff() >= 15) MGR.trophies[3]++;
  if (pos == 1 && leagueLosses() == 0) MGR.trophies[4]++;
  for (int k = 0; k < MGR.nsq; k++) {
    MPl *p = &MGR.sq[k];
    p->age++;
    if (p->age < 24) p->ovr = clampi(p->ovr + rn(1, 4), 40, 92);
    else if (p->age < 30) p->ovr = clampi(p->ovr + rn(-1, 2), 40, 92);
    else p->ovr = clampi(p->ovr - rn(0, 3), 40, 92);
    p->price = priceOf(p);
  }
  MGR.sel2 = 1;
  mgrSave();
}

void mgrBeginSeason(void);
void mgrBeginSeason(void) {
  MGR.season++; MGR.sel2 = 0;
  for (int k = 0; k < MGR.nsq; k++) { MGR.sq[k].goals = 0; MGR.sq[k].games = 0; MGR.sq[k].fit = 100; }
  fillXI(0);
  genMarket();
  MGR.boardGoal = clampi(7 - (avgXI() - 55) / 4, 1, 10);
  leagueStart(MGR.team);
  SCENE = SC_MGR; MGR.sub = 0;
  mgrSave();
}

static void subList(void) {
  char b[80];
  bg(); ctxt(8, "ELENCO", C_YELLOW, 240);
  txt(20, 24, "POS NOME      OVR IDD GOL JGS FIT", C_GREY);
  int rows = 12;
  if (hit(B_UP)) hubSel = (hubSel > 0) ? hubSel - 1 : MGR.nsq - 1;
  if (hit(B_DOWN)) hubSel = (hubSel < MGR.nsq - 1) ? hubSel + 1 : 0;
  listTop = clampi(hubSel - rows / 2, 0, MGR.nsq > rows ? MGR.nsq - rows : 0);
  for (int i = listTop; i < MGR.nsq && i < listTop + rows; i++) {
    const MPl *p = &MGR.sq[i];
    int y = 40 + (i - listTop) * 16;
    if (i == hubSel) box(14, y - 2, 452, 15, C_SEL);
    snprintf(b, sizeof b, "%s %-9s %3d %3d %3d %3d %3d%%", mgrPosName(p->pos), nameOf(p->name), p->ovr, p->age, p->goals, p->games, p->fit);
    txt(20, y, b, (i == hubSel) ? C_BLACK : ((p->fit < 60) ? C_ORANGE : C_WHITE));
  }
  ctxt(240, "TRIANGULO volta", C_GREY, 240);
  if (hit(B_TRIANGLE)) { MGR.sub = 0; hubSel = 2; }
}

static void subLineup(void) {
  char b[80];
  bg(); ctxt(6, "ESCALACAO", C_YELLOW, 240);
  snprintf(b, sizeof b, "FORMACAO %s", formName(2, MGR.form)); ctxt(20, b, C_GREY, 120);
  for (int i = 0; i < 11; i++) {
    int y = 36 + i * 16, role = formRole(2, MGR.form, i);
    int x = MGR.xi[i];
    if (lsStage == 0 && i == lsSlot) box(8, y - 2, 230, 15, C_SEL);
    Col c = (lsStage == 0 && i == lsSlot) ? C_BLACK : C_WHITE;
    if (x >= 0) snprintf(b, sizeof b, "%s %-8s %2d", mgrPosName(role), nameOf(MGR.sq[x].name), MGR.sq[x].ovr);
    else snprintf(b, sizeof b, "%s ---", mgrPosName(role));
    txt(14, y, b, c);
  }
  if (lsStage == 0) {
    if (hit(B_UP)) lsSlot = (lsSlot > 0) ? lsSlot - 1 : 10;
    if (hit(B_DOWN)) lsSlot = (lsSlot < 10) ? lsSlot + 1 : 0;
    if (hit(B_CROSS)) { lsStage = 1; lsCand = 0; }
    if (hit(B_SQUARE)) fillXI(1);
    if (hit(B_TRIANGLE)) { MGR.sub = 0; hubSel = 3; }
    ctxt(240, "X troca  [] automatico  TRIANGULO volta", C_GREY, 240);
  } else {
    int rows = 11;
    int top = clampi(lsCand - rows / 2, 0, MGR.nsq > rows ? MGR.nsq - rows : 0);
    for (int i = top; i < MGR.nsq && i < top + rows; i++) {
      const MPl *p = &MGR.sq[i];
      int y = 36 + (i - top) * 16;
      int inxi = 0; for (int k = 0; k < 11; k++) if (MGR.xi[k] == i) inxi = 1;
      if (i == lsCand) box(250, y - 2, 224, 15, C_SEL);
      snprintf(b, sizeof b, "%s %-8s %2d %3d%%", mgrPosName(p->pos), nameOf(p->name), p->ovr, p->fit);
      txt(256, y, b, (i == lsCand) ? C_BLACK : (inxi ? C_GREEN : C_WHITE));
    }
    if (hit(B_UP)) lsCand = (lsCand > 0) ? lsCand - 1 : MGR.nsq - 1;
    if (hit(B_DOWN)) lsCand = (lsCand < MGR.nsq - 1) ? lsCand + 1 : 0;
    if (hit(B_TRIANGLE)) lsStage = 0;
    if (hit(B_CROSS)) {
      int other = -1; for (int k = 0; k < 11; k++) if (MGR.xi[k] == lsCand) other = k;
      if (other >= 0) MGR.xi[other] = MGR.xi[lsSlot];
      MGR.xi[lsSlot] = lsCand; lsStage = 0;
    }
    ctxt(240, "X escolhe jogador (verde = ja titular)", C_GREY, 240);
  }
}

static void subTrain(void) {
  bg(); ctxt(10, "TREINO", C_YELLOW, 240);
  static const char *desc[5] = { "Resistencia e melhora geral leve", "Atacantes e meias evoluem mais", "Defensores e goleiros evoluem mais", "Meias evoluem mais", "Todos recuperam o folego" };
  for (int i = 0; i < 5; i++) {
    int y = 50 + i * 28;
    if (trSel == i) box(100, y - 4, 280, 22, C_SEL);
    char b[48]; snprintf(b, sizeof b, "%s%s", TRAINN[i], (MGR.train == i) ? "  (ATUAL)" : "");
    ctxt(y, b, (trSel == i) ? C_BLACK : C_WHITE, 240);
  }
  ctxt(200, desc[trSel], C_GREY, 240);
  ctxt(240, "X escolhe   TRIANGULO volta", C_GREY, 240);
  if (hit(B_UP)) trSel = (trSel > 0) ? trSel - 1 : 4;
  if (hit(B_DOWN)) trSel = (trSel < 4) ? trSel + 1 : 0;
  if (hit(B_CROSS)) MGR.train = trSel;
  if (hit(B_TRIANGLE)) { MGR.sub = 0; hubSel = 5; }
}

static void subMarket(void) {
  char b[96], m[24];
  bg(); ctxt(8, tab == 0 ? "MERCADO - COMPRAR" : "MERCADO - VENDER", C_YELLOW, 240);
  moneyStr(m, sizeof m, MGR.money); snprintf(b, sizeof b, "CAIXA %s", m); ctxt(24, b, C_GREEN, 240);
  int n = (tab == 0) ? MGR.nmarket : MGR.nsq;
  if (mSel >= n) mSel = 0;
  int rows = 10, top = clampi(mSel - rows / 2, 0, n > rows ? n - rows : 0);
  for (int i = top; i < n && i < top + rows; i++) {
    const MPl *p = (tab == 0) ? &MGR.market[i] : &MGR.sq[i];
    int y = 42 + (i - top) * 16;
    int price = (tab == 0) ? p->price : p->price * 6 / 10;
    if (i == mSel) box(14, y - 2, 452, 15, C_SEL);
    moneyStr(m, sizeof m, price);
    snprintf(b, sizeof b, "%s %-9s OVR %2d IDD %2d  %s", mgrPosName(p->pos), nameOf(p->name), p->ovr, p->age, m);
    txt(20, y, b, (i == mSel) ? C_BLACK : C_WHITE);
  }
  if (msgLine[0]) ctxt(208, msgLine, C_YELLOW, 240);
  ctxt(224, "ESQ/DIR troca aba   X confirma", C_GREY, 240);
  ctxt(240, "TRIANGULO volta", C_GREY, 240);
  if (hit(B_UP)) mSel = (mSel > 0) ? mSel - 1 : n - 1;
  if (hit(B_DOWN)) mSel = (mSel < n - 1) ? mSel + 1 : 0;
  if (hit(B_LEFT) || hit(B_RIGHT)) { tab = !tab; mSel = 0; msgLine[0] = 0; }
  if (hit(B_TRIANGLE)) { MGR.sub = 0; hubSel = 6; msgLine[0] = 0; }
  if (hit(B_CROSS)) {
    if (tab == 0 && MGR.nmarket > 0) {
      MPl p = MGR.market[mSel];
      if (MGR.nsq >= MAXSQ) snprintf(msgLine, sizeof msgLine, "ELENCO CHEIO");
      else if (MGR.money < p.price) snprintf(msgLine, sizeof msgLine, "DINHEIRO INSUFICIENTE");
      else {
        MGR.money -= p.price; MGR.sq[MGR.nsq++] = p;
        for (int k = mSel; k < MGR.nmarket - 1; k++) MGR.market[k] = MGR.market[k + 1];
        MGR.nmarket--; snprintf(msgLine, sizeof msgLine, "CONTRATADO: %s", nameOf(p.name));
      }
    } else if (tab == 1) {
      int gks = 0; for (int k = 0; k < MGR.nsq; k++) if (MGR.sq[k].pos == 0) gks++;
      if (MGR.nsq <= 14) snprintf(msgLine, sizeof msgLine, "ELENCO MINIMO: 14");
      else if (MGR.sq[mSel].pos == 0 && gks <= 1) snprintf(msgLine, sizeof msgLine, "PRECISA DE UM GOLEIRO");
      else {
        MGR.money += MGR.sq[mSel].price * 6 / 10;
        snprintf(msgLine, sizeof msgLine, "VENDIDO: %s", nameOf(MGR.sq[mSel].name));
        for (int k = mSel; k < MGR.nsq - 1; k++) MGR.sq[k] = MGR.sq[k + 1];
        MGR.nsq--;
        for (int k = 0; k < 11; k++) { if (MGR.xi[k] == mSel) MGR.xi[k] = -1; else if (MGR.xi[k] > mSel) MGR.xi[k]--; }
        fillXI(0);
      }
    }
  }
}

static void subFinance(void) {
  char b[96], m[24];
  bg(); ctxt(10, "FINANCAS E DIRETORIA", C_YELLOW, 240);
  moneyStr(m, sizeof m, MGR.money); snprintf(b, sizeof b, "Caixa: %s", m); txt(60, 44, b, (MGR.money < 0) ? C_RED : C_GREEN);
  moneyStr(m, sizeof m, MGR.lastIncome); snprintf(b, sizeof b, "Ultima receita de jogo: %s", m); txt(60, 62, b, C_WHITE);
  moneyStr(m, sizeof m, MGR.lastWages); snprintf(b, sizeof b, "Ultimos salarios: %s", m); txt(60, 78, b, C_WHITE);
  moneyStr(m, sizeof m, MGR.lastPrize); snprintf(b, sizeof b, "Ultimo premio de temporada: %s", m); txt(60, 94, b, C_WHITE);
  snprintf(b, sizeof b, "Meta da diretoria: terminar no top %d", MGR.boardGoal); txt(60, 118, b, C_YELLOW);
  snprintf(b, sizeof b, "Posicao atual: %do", leagueUserPos()); txt(60, 134, b, C_WHITE);
  snprintf(b, sizeof b, "Media do time titular: %d", avgXI()); txt(60, 150, b, C_WHITE);
  ctxt(190, tip(), C_GREY, 240);
  ctxt(240, "TRIANGULO volta", C_GREY, 240);
  if (hit(B_TRIANGLE)) { MGR.sub = 0; hubSel = 7; }
}

static void subTrophies(void) {
  char b[48];
  bg(); ctxt(10, "SALA DE TROFEUS", C_YELLOW, 240);
  const short *ic[5] = { FR_TROPHY_CUP, FR_TROPHY_MEDAL, FR_TROPHY_BOOT, FR_TROPHY_BALLON, FR_TROPHY_CHAMP };
  static const char *nm[5] = { "TITULOS", "PODIOS", "ARTILHEIRO", "SALDO +15", "INVICTO" };
  for (int i = 0; i < 5; i++) {
    int x = 50 + i * 95;
    drawFrame(ic[i], x, 120);
    snprintf(b, sizeof b, "x%d", MGR.trophies[i]);
    ctxt(128, b, C_WHITE, x);
    ctxt(144, nm[i], C_GREY, x);
  }
  ctxt(240, "TRIANGULO volta", C_GREY, 240);
  if (hit(B_TRIANGLE)) { MGR.sub = 0; hubSel = 9; }
}

static void subSeasonEnd(void) {
  char b[96], m[24];
  if (!MGR.sel2) endSeason();
  bg();
  int pos = leagueUserPos();
  ctxt(12, "FIM DE TEMPORADA", C_YELLOW, 240);
  if (pos == 1) { drawFrame(FR_TROPHY_CUP, 240, 80); ctxt(88, "CAMPEAO!", C_YELLOW, 240); }
  snprintf(b, sizeof b, "Terminou em %do lugar", pos); ctxt(112, b, C_WHITE, 240);
  moneyStr(m, sizeof m, MGR.lastPrize); snprintf(b, sizeof b, "Premio: %s", m); ctxt(130, b, C_GREEN, 240);
  snprintf(b, sizeof b, "Meta da diretoria (top %d): %s", MGR.boardGoal, (pos <= MGR.boardGoal) ? "CUMPRIDA" : "NAO CUMPRIDA");
  ctxt(148, b, (pos <= MGR.boardGoal) ? C_GREEN : C_RED, 240);
  snprintf(b, sizeof b, "Artilheiro do clube: %d gols", MGR.scorerBest); ctxt(166, b, C_WHITE, 240);
  ctxt(190, "Os jogadores envelheceram e evoluiram", C_GREY, 240);
  ctxt(230, "X nova temporada", C_WHITE, 240);
  if (hit(B_CROSS)) mgrBeginSeason();
}

static void subCoach(void) {
  bg(); ctxt(10, "ESCOLHA SEU TREINADOR", C_YELLOW, 240);
  for (int i = 0; i < 5; i++) {
    int x = 40 + i * 84;
    if (MGR.coach == i) box(x - 4, 70, 72, 72, C_SEL);
    if (T_COACH >= 0) plat_blit(T_COACH, i * 64, 0, 64, 64, x, 74, 255, 255, 255, 255);
  }
  ctxt(160, "ESQ/DIR escolhe    X confirma", C_GREY, 240);
  if (hit(B_LEFT)) MGR.coach = (MGR.coach + 4) % 5;
  if (hit(B_RIGHT)) MGR.coach = (MGR.coach + 1) % 5;
  if (hit(B_CROSS)) { MGR.sub = 0; hubSel = 0; mgrSave(); }
}

void sceneMgr(void) {
  if (!MGR.active) { SCENE = SC_MENU; return; }
  if (MGR.sub == 10) { subCoach(); return; }
  if (leagueIsDone() && MGR.sub != 11 && MGR.sub != 6) MGR.sub = 11;
  switch (MGR.sub) {
    case 1: subList(); return;
    case 2: subLineup(); return;
    case 3: {
      bg(); ctxt(14, "TATICA DO TIME", C_YELLOW, 240);
      if (tacEdit(&MGR.tac, &MGR.form, 2, 50, &tacSel2)) { fillXI(0); MGR.sub = 0; hubSel = 4; }
      ctxt(220, "As mudancas valem na proxima partida", C_GREY, 240);
      return;
    }
    case 4: subTrain(); return;
    case 5: subMarket(); return;
    case 7: subFinance(); return;
    case 8: subTrophies(); return;
    case 11: subSeasonEnd(); return;
    default: break;
  }
  /* hub */
  char b[96];
  if (hubSelDebug >= 0) { hubSel = hubSelDebug; hubSelDebug = -1; }
  bg(); ctxt(6, "MODO MANAGER", C_YELLOW, 240);
  drawHubHeader();
  static const char *items[11] = { "PROXIMO JOGO", "SIMULAR JOGO", "ELENCO", "ESCALACAO", "TATICA", "TREINO", "MERCADO", "FINANCAS", "TABELA", "TROFEUS", "SALVAR E SAIR" };
  for (int i = 0; i < 11; i++) {
    int y = 26 + i * 18;
    if (hubSel == i) box(150, y - 3, 310, 17, C_SEL);
    ctxt(y, items[i], (hubSel == i) ? C_BLACK : C_WHITE, 305);
  }
  snprintf(b, sizeof b, "RODADA %d/11  vs %s", leagueRoundNo(), TEAMS[leagueNextOpp()].name);
  ctxt(228, b, C_WHITE, 240);
  ctxt(243, tip(), C_GREY, 240);
  if (hit(B_UP)) hubSel = (hubSel > 0) ? hubSel - 1 : 10;
  if (hit(B_DOWN)) hubSel = (hubSel < 10) ? hubSel + 1 : 0;
  if (hit(B_CROSS)) {
    switch (hubSel) {
      case 0: fillXI(0); leaguePlayMatch(); break;
      case 1: fillXI(0); { int a = avgXI(); TEAMS[MGR.team].str = clampi((a - 45 + 3) / 6, 1, 5); } leagueSimUser(); break;
      case 2: MGR.sub = 1; hubSel = 0; break;
      case 3: fillXI(0); MGR.sub = 2; lsSlot = 0; lsStage = 0; break;
      case 4: MGR.sub = 3; tacSel2 = 0; break;
      case 5: MGR.sub = 4; trSel = MGR.train; break;
      case 6: MGR.sub = 5; mSel = 0; tab = 0; msgLine[0] = 0; break;
      case 7: MGR.sub = 7; break;
      case 8: SCENE = SC_LEAGUE; break;
      case 9: MGR.sub = 8; break;
      case 10: mgrSave(); MGR.active = 0; SCENE = SC_MENU; break;
    }
  }
  if (hit(B_TRIANGLE)) { mgrSave(); MGR.active = 0; SCENE = SC_MENU; }
}
