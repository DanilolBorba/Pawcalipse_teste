"""Sintetiza os efeitos sonoros do PawPocalypse (WAV 22050 Hz, 16 bits, mono)."""
import math, random, struct, sys, wave, os

TAXA = 22050
random.seed(7)
destino = sys.argv[1]

def amostras(seg): return int(seg * TAXA)

def salvar(nome, sinal, pico=0.8):
    m = max(1e-9, max(abs(v) for v in sinal))
    with wave.open(os.path.join(destino, nome + '.wav'), 'wb') as w:
        w.setnchannels(1); w.setsampwidth(2); w.setframerate(TAXA)
        w.writeframes(b''.join(struct.pack('<h', int(v / m * pico * 32767)) for v in sinal))

def env(t, ataque, decai):            # ataque linear e queda exponencial
    return (t / ataque if t < ataque else 1.0) * math.exp(-max(0.0, t - ataque) * decai)

def fade(sinal, seg=0.01):            # tira o "clique" do começo e do fim
    n = amostras(seg)
    for i in range(min(n, len(sinal))):
        sinal[i] *= i / n; sinal[-1 - i] *= i / n
    return sinal

class Filtro:                         # passa-faixa (biquad) para formantes de voz
    def __init__(s, freq, q): s.ajustar(freq, q); s.x1 = s.x2 = s.y1 = s.y2 = 0.0
    def ajustar(s, freq, q):
        w = 2 * math.pi * freq / TAXA; a = math.sin(w) / (2 * q)
        s.b0, s.b2, s.a0, s.a1, s.a2 = a, -a, 1 + a, -2 * math.cos(w), 1 - a
    def __call__(s, x):
        y = (s.b0 * x + s.b2 * s.x2 - s.a1 * s.y1 - s.a2 * s.y2) / s.a0
        s.x2, s.x1, s.y2, s.y1 = s.x1, x, s.y1, y
        return y

def passa_baixa(sinal, alfa):
    y = 0.0; out = []
    for v in sinal: y += alfa * (v - y); out.append(y)
    return out

def voz(dur, f0, formantes, rouquidao=0.0, vib=0.0):
    """Dente-de-serra com altura f0(t), passada por formantes(t) -> [(freq, ganho)]."""
    filtros = [Filtro(800, 6) for _ in range(3)]
    fase = 0.0; out = []
    for i in range(amostras(dur)):
        t = i / TAXA
        f = f0(t) * (1 + vib * math.sin(2 * math.pi * 6 * t))
        fase = (fase + f / TAXA) % 1.0
        x = 2 * fase - 1 + rouquidao * random.uniform(-1, 1)
        v = 0.0
        for filtro, (ff, g) in zip(filtros, formantes(t)):
            filtro.ajustar(ff, 5); v += g * filtro(x)
        out.append(v)
    return out

def tom(dur, freq, forma='seno', decai=6.0, ataque=0.005):
    out = []; fase = 0.0
    for i in range(amostras(dur)):
        t = i / TAXA; fase = (fase + freq / TAXA) % 1.0
        if forma == 'seno': x = math.sin(2 * math.pi * fase)
        elif forma == 'quadrada': x = 0.6 if fase < 0.5 else -0.6
        else: x = 4 * abs(fase - 0.5) - 1
        out.append(x * env(t, ataque, decai))
    return out

def misturar(*partes):
    n = max(len(p) for _, p in partes); out = [0.0] * n
    for inicio, p in partes:
        k = amostras(inicio)
        for i, v in enumerate(p):
            if k + i < n: out[k + i] += v
            else: out.append(v)
    return out

def interp(pontos, t):                 # curva por pontos (t, valor)
    for (t0, v0), (t1, v1) in zip(pontos, pontos[1:]):
        if t <= t1: return v0 + (v1 - v0) * (t - t0) / (t1 - t0)
    return pontos[-1][1]

# clique de botão: blip curto e agudo
salvar('clique', fade(misturar((0, tom(0.06, 1400, decai=60)), (0, [0.3 * v for v in tom(0.06, 2100, decai=80)]))), 0.6)

# cachorro colocado no campo: "pof" grave
pof = []; fase = 0.0
for i in range(amostras(0.18)):
    t = i / TAXA; fase += (320 - 1100 * t) / TAXA
    pof.append(math.sin(2 * math.pi * fase) * env(t, 0.003, 22) + 0.15 * random.uniform(-1, 1) * env(t, 0.001, 60))
salvar('colocar', fade(pof), 0.7)

# petisco coletado: "bling" de duas notas
salvar('petisco', fade(misturar((0, tom(0.09, 1319, decai=25)), (0.07, tom(0.28, 1760, decai=12)),
                                (0.07, [0.3 * v for v in tom(0.28, 3520, decai=18)]))), 0.6)

# cuspe de baba: ruído molhado + "ptu" que desce
ruido = passa_baixa([random.uniform(-1, 1) for _ in range(amostras(0.2))], 0.35)
cuspe = []; fase = 0.0
for i in range(amostras(0.2)):
    t = i / TAXA; fase += (700 - 2400 * t) / TAXA
    cuspe.append(ruido[i] * env(t, 0.004, 28) * 1.4 + 0.6 * math.sin(2 * math.pi * fase) * env(t, 0.002, 30))
salvar('cuspe', fade(cuspe), 0.55)

# latido: "au!" com a altura caindo e formantes de "a" para "u"
latido = voz(0.24, lambda t: 420 - 600 * t, lambda t: [(interp([(0, 950), (0.24, 520)], t), 1.0),
                                                       (interp([(0, 1500), (0.24, 900)], t), 0.5), (2600, 0.2)], rouquidao=0.25)
latido = [v * env(i / TAXA, 0.008, 9) for i, v in enumerate(latido)]
salvar('latido', fade(latido), 0.8)

# explosão do Rottweiler: estrondo de ruído grave
ruido = [random.uniform(-1, 1) for _ in range(amostras(0.9))]
estrondo = passa_baixa(ruido, 0.08)
boom = [estrondo[i] * env(i / TAXA, 0.004, 5) * 3 + 0.8 * math.sin(2 * math.pi * 55 * i / TAXA) * env(i / TAXA, 0.002, 7) for i in range(len(ruido))]
salvar('explosao', fade(boom), 0.85)

# zumbido: gemido grave de zumbi ("uuurgh") com dois passos arrastados
gemido = voz(1.1, lambda t: 118 + 14 * math.sin(2 * math.pi * 1.7 * t) - 25 * t,
             lambda t: [(interp([(0, 420), (0.5, 560), (1.1, 380)], t), 1.0), (interp([(0, 800), (1.1, 700)], t), 0.45), (2300, 0.08)],
             rouquidao=0.6, vib=0.02)
gemido = [v * min(1, (i / TAXA) / 0.18) * (1 - (i / TAXA) / 1.1) ** 0.8 for i, v in enumerate(gemido)]
passo = passa_baixa([random.uniform(-1, 1) * env(i / TAXA, 0.01, 18) for i in range(amostras(0.16))], 0.12)
salvar('zumbido', fade(misturar((0, gemido), (0.25, [0.5 * v for v in passo]), (0.7, [0.4 * v for v in passo]))), 0.6)

# mordida: rosnado curto e o "nhac"
rosnado = voz(0.4, lambda t: 95 + 20 * math.sin(2 * math.pi * 7 * t), lambda t: [(450, 1.0), (900, 0.4), (1600, 0.2)], rouquidao=0.8)
rosnado = [v * (0.6 + 0.4 * math.sin(2 * math.pi * 28 * i / TAXA)) * env(i / TAXA, 0.03, 4) for i, v in enumerate(rosnado)]
nhac = [random.uniform(-1, 1) * env(i / TAXA, 0.001, 70) for i in range(amostras(0.08))]
salvar('mordida', fade(misturar((0, rosnado), (0.22, [0.35 * v for v in passa_baixa(nhac, 0.5)]))), 0.7)

# gato derrotado: miado que cai e some
morre = voz(0.7, lambda t: interp([(0, 720), (0.15, 820), (0.7, 220)], t),
            lambda t: [(interp([(0, 1800), (0.7, 500)], t), 1.0), (1200, 0.3), (2800, 0.15)], rouquidao=0.35, vib=0.05)
morre = [v * min(1, (i / TAXA) / 0.03) * (1 - (i / TAXA) / 0.7) ** 1.3 for i, v in enumerate(morre)]
salvar('gato_morre', fade(morre), 0.55)

# rugido do Rei: rosnado longo, grave e que cresce
rugido = voz(1.3, lambda t: interp([(0, 70), (0.5, 90), (1.3, 55)], t), lambda t: [(380, 1.0), (760, 0.5), (1300, 0.25)], rouquidao=0.9)
rugido = [v * (0.7 + 0.3 * math.sin(2 * math.pi * 23 * i / TAXA)) * min(1, (i / TAXA) / 0.25) * (1 - (i / TAXA) / 1.3) ** 0.6 for i, v in enumerate(rugido)]
sub = [0.5 * math.sin(2 * math.pi * 40 * i / TAXA) * min(1, (i / TAXA) / 0.3) * (1 - (i / TAXA) / 1.3) for i in range(len(rugido))]
salvar('rugido', fade(misturar((0, rugido), (0, sub))), 0.85)

# alerta da horda: sirene mecânica (tipo antiaérea) que "liga" subindo, segura e desce;
# grave e encorpada (dois tons de terça menor, cheios de harmônicos) e suavizada
DUR = 2.8
def altura(t):
    if t < 1.0: return 150 + 370 * (1 - (1 - t / 1.0) ** 2)
    if t < 1.7: return 520 + 8 * math.sin(2 * math.pi * 3 * t)
    return 520 - 250 * ((t - 1.7) / (DUR - 1.7)) ** 1.5
sirene = []; fases = [0.0, 0.0]
for i in range(amostras(DUR)):
    t = i / TAXA; x = 0.0
    for k, mult in enumerate((1.0, 1.19)):
        fases[k] = (fases[k] + altura(t) * mult * (1 + 0.004 * math.sin(2 * math.pi * 5 * t)) / TAXA) % 1.0
        x += sum(math.sin(2 * math.pi * h * fases[k]) / h for h in range(1, 6)) * (1.0 if k == 0 else 0.7)
    sirene.append(x * min(1, t / 0.35) * min(1, (DUR - t) / 0.6))
salvar('alerta', fade(passa_baixa(sirene, 0.22)), 0.5)

# vitória: arpejo subindo
notas = [523, 659, 784, 1047]
salvar('vitoria', fade(misturar(*[(k * 0.11, tom(0.5 if k == 3 else 0.14, f, 'quadrada', decai=4 if k == 3 else 14)) for k, f in enumerate(notas)])), 0.55)

# derrota: notas descendo, a última longa
notas = [392, 370, 349, 330]
salvar('derrota', fade(misturar(*[(k * 0.28, tom(0.9 if k == 3 else 0.3, f, 'triangular', decai=2 if k == 3 else 6)) for k, f in enumerate(notas)])), 0.6)
# carta in-game: "toc" de madeira, mais grave e suave que o clique do menu
salvar('carta', fade(misturar((0, tom(0.08, 620, decai=45)), (0, [0.4 * v for v in tom(0.08, 1240, decai=60)]),
                              (0, [0.2 * random.uniform(-1, 1) * env(i / TAXA, 0.001, 120) for i in range(amostras(0.03))]))), 0.55)

# iniciar a partida: arpejo rápido subindo com um "vush"
vush = [random.uniform(-1, 1) for _ in range(amostras(0.35))]
filtro = Filtro(500, 2); vush = [filtro(v) * min(1, (i / TAXA) / 0.2) * (1 - (i / TAXA) / 0.35) for i, v in enumerate(vush) if filtro.ajustar(500 + 6000 * i / len(vush), 2) is None]
notas = [392, 523, 659, 784]
salvar('iniciar', fade(misturar((0, [0.5 * v for v in vush]), *[(0.05 + k * 0.06, tom(0.35 if k == 3 else 0.09, f, 'triangular', decai=6 if k == 3 else 20)) for k, f in enumerate(notas)])), 0.6)

# pá: a lâmina entra na terra ("tchk") e raspa ("shhh")
raspa = [random.uniform(-1, 1) for _ in range(amostras(0.3))]
filtro = Filtro(1400, 1.5); raspa = [filtro(v) * env(i / TAXA, 0.02, 9) for i, v in enumerate(raspa)]
tchk = [math.sin(2 * math.pi * 140 * i / TAXA) * env(i / TAXA, 0.002, 35) + 0.5 * random.uniform(-1, 1) * env(i / TAXA, 0.001, 80) for i in range(amostras(0.1))]
salvar('pa', fade(misturar((0, tchk), (0.04, [1.6 * v for v in raspa]))), 0.65)

# luva: "fuíp" (ar subindo) e um "plop" ao pegar ou soltar o cachorro
fuip = [random.uniform(-1, 1) for _ in range(amostras(0.16))]
filtro = Filtro(400, 3); fuip = [filtro(v) * math.sin(math.pi * i / len(fuip)) for i, v in enumerate(fuip) if filtro.ajustar(400 + 2600 * i / len(fuip), 3) is None]
plop = []; fase = 0.0
for i in range(amostras(0.1)):
    t = i / TAXA; fase += (500 + 900 * t) / TAXA; plop.append(math.sin(2 * math.pi * fase) * env(t, 0.003, 40))
salvar('luva', fade(misturar((0, [2.0 * v for v in fuip]), (0.12, plop))), 0.6)

# gerador de ração: "clanc" da máquina e grãos de ração caindo na tigela
clanc = [math.sin(2 * math.pi * 170 * i / TAXA) * env(i / TAXA, 0.002, 25) + 0.4 * math.sin(2 * math.pi * 523 * i / TAXA) * env(i / TAXA, 0.002, 30) for i in range(amostras(0.2))]
graos = []
for k in range(7):
    inicio = 0.12 + k * 0.045 + random.uniform(0, 0.02); f = random.uniform(2500, 4200)
    graos.append((inicio, [0.9 * math.sin(2 * math.pi * f * i / TAXA) * env(i / TAXA, 0.001, 140) for i in range(amostras(0.03))]))
salvar('racao', fade(misturar((0, clanc), *graos)), 0.5)
print('ok')
