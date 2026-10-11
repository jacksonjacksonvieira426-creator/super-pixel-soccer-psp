#ifndef TEAMS_H
#define TEAMS_H
#define NTEAMS 25
#define NCLUBS 24
#define T_LENDAS 24
#define TEAMS_INIT { \
  { "TRICOLOR", "TRI", KIT_C1, {0}, 0, 4, { 150, 30, 60, 255 }, KIT_GK }, \
  { "VERDAO", "VRD", KIT_C2, { KIT_C2, KIT_NPALM }, 2, 5, { 20, 130, 60, 255 }, KIT_GK }, \
  { "CRUZ MALT", "CMA", KIT_C3, {0}, 0, 3, { 60, 60, 60, 255 }, KIT_GK }, \
  { "PEIXE", "PXE", KIT_C4, {0}, 0, 4, { 230, 230, 230, 255 }, KIT_GK }, \
  { "MACACA", "MAC", KIT_C5, {0}, 0, 2, { 220, 220, 220, 255 }, KIT_GK }, \
  { "ALVINEGRO", "ALV", KIT_C6, {0}, 0, 3, { 110, 110, 110, 255 }, KIT_GK }, \
  { "LEAO", "LEA", KIT_C7, {0}, 0, 3, { 200, 30, 30, 255 }, KIT_GK }, \
  { "ESMERALDA", "ESM", KIT_C8, {0}, 0, 2, { 30, 150, 80, 255 }, KIT_GK }, \
  { "RUBRO NEGRO", "RUB", KIT_NFLAM, { KIT_NFLAM }, 1, 5, { 200, 30, 30, 255 }, KIT_GKO }, \
  { "DIABOS", "DIA", KIT_NMANU, { KIT_NMANU }, 1, 4, { 210, 30, 30, 255 }, KIT_GKG }, \
  { "ZEBRAS", "ZEB", KIT_NJUVE9, { KIT_NJUVE9, KIT_NJUVE10 }, 2, 5, { 235, 235, 235, 255 }, KIT_GKP }, \
  { "MERENGUES", "MER", KIT_NREAL7, { KIT_NREAL7, KIT_NREAL7B }, 2, 5, { 245, 245, 245, 255 }, KIT_GKG }, \
  { "TROVAO", "TRV", KIT_N0A, { KIT_N0A, KIT_N0B }, 2, 4, { 30, 70, 210, 255 }, KIT_GKG }, \
  { "DRAGOES", "DRG", KIT_N1A, { KIT_N1A, KIT_N1B }, 2, 4, { 200, 35, 35, 255 }, KIT_GKO }, \
  { "RELAMPAGO", "REL", KIT_N2A, { KIT_N2A, KIT_N2B }, 2, 3, { 255, 215, 0, 255 }, KIT_GKP }, \
  { "FALCOES", "FAL", KIT_N3A, { KIT_N3A, KIT_N3B }, 2, 3, { 245, 245, 245, 255 }, KIT_GKG }, \
  { "SOLAR", "SOL", KIT_N4A, { KIT_N4A, KIT_N4B }, 2, 3, { 255, 140, 20, 255 }, KIT_GKO }, \
  { "LOBOS", "LOB", KIT_N5A, { KIT_N5A, KIT_N5B }, 2, 4, { 125, 125, 135, 255 }, KIT_GKP }, \
  { "TUBAROES", "TUB", KIT_N6A, { KIT_N6A, KIT_N6B }, 2, 2, { 20, 150, 205, 255 }, KIT_GKG }, \
  { "ONCAS", "ONC", KIT_N7A, { KIT_N7A, KIT_N7B }, 2, 3, { 235, 190, 60, 255 }, KIT_GKO }, \
  { "CORSARIOS", "COR", KIT_N8A, { KIT_N8A, KIT_N8B }, 2, 4, { 35, 35, 40, 255 }, KIT_GKP }, \
  { "VULCAO", "VUL", KIT_N9A, { KIT_N9A, KIT_N9B }, 2, 5, { 215, 60, 25, 255 }, KIT_GKG }, \
  { "GALAXIA", "GAL", KIT_N10A, { KIT_N10A, KIT_N10B }, 2, 5, { 105, 50, 175, 255 }, KIT_GKO }, \
  { "BOREAL", "BOR", KIT_N11A, { KIT_N11A, KIT_N11B }, 2, 2, { 60, 195, 155, 255 }, KIT_GKP }, \
  { "LENDAS", "LEN", KIT_C4, { KIT_PELE, KIT_MESSI, KIT_NEYMAR, KIT_CR7, KIT_MARA }, 5, 5, { 255, 215, 60, 255 }, KIT_GK } }
#endif
