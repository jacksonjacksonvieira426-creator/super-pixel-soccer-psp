#ifndef STADIUMS_H
#define STADIUMS_H
/* geometria (em pixels da tela 480x272) de cada estadio de imagem: y nas ancoras V7, bordas esq/dir no topo e no fundo */
typedef struct { float y[7], l0, r0, l1, r1; } StGeo;
static const float V7[7] = { 0.0f, 0.203f, 0.365f, 0.5f, 0.635f, 0.797f, 1.0f };
static const StGeo STG[6] = {
  { { 104.36f, 123.72f, 143.08f, 163.62f, 184.40f, 207.31f, 244.14f }, 72.05f, 407.48f, 16.06f, 464.17f }, /* maracana */
  { { 131.28f, 140.49f, 148.75f, 155.83f, 169.29f, 185.35f, 230.21f }, 114.57f, 365.67f, 81.26f, 399.92f }, /* palmeiras */
  { { 105.54f, 126.32f, 141.67f, 162.92f, 184.40f, 197.62f, 246.26f }, 70.87f, 411.02f, 8.74f, 471.97f }, /* kop */
  { { 95.62f, 120.42f, 141.19f, 162.44f, 188.18f, 217.69f, 255.47f }, 81.50f, 402.76f, 22.44f, 459.45f }, /* real */
  { { 85.94f, 106.49f, 129.39f, 151.82f, 174.01f, 208.01f, 244.38f }, 98.74f, 379.13f, 49.61f, 430.39f }, /* united */
  { { 133.64f, 142.61f, 150.17f, 157.25f, 170.47f, 186.29f, 230.21f }, 115.04f, 363.07f, 82.68f, 398.03f }, /* barca */
};
#endif
