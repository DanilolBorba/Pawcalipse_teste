# PawPocalypse 🐾 — Cães vs. Gatos Zumbis

Em um mundo pós-apocalíptico onde os gatos foram infectados por um vírus misterioso e se transformaram em hordas famintas de Gatos Zumbis, a última esperança da humanidade está nas patas dos seus companheiros mais leais: os Cachorros. Organizados por raças, cada uma com habilidades únicas e papéis estratégicos, os cães se uniram para defender os últimos territórios e deter a invasão felina.

Jogo de defesa em pixel-art, no estilo *Plants vs. Zombies 2*, feito em **C puro (C99) com a biblioteca [raylib](https://www.raylib.com/)** para a disciplina de programação.

---

## Sumário

1. [Como compilar e jogar](#1-como-compilar-e-jogar)
2. [Como jogar](#2-como-jogar)
3. [Conteúdo do jogo](#3-conteúdo-do-jogo)
4. [Onde está cada requisito do trabalho](#4-onde-está-cada-requisito-do-trabalho)
5. [Mapa do código](#5-mapa-do-código)
6. [Pastas do projeto](#6-pastas-do-projeto)
7. [Teclas de teste](#7-teclas-de-teste)
8. [Lista de todas as funções](#8-lista-de-todas-as-funções)

---

## 1. Como compilar e jogar

É preciso ter a **raylib** instalada e um compilador C (gcc ou clang).

| Sistema | Instalar a raylib |
|---|---|
| macOS | `brew install raylib` |
| Linux (Debian/Ubuntu) | `sudo apt install libraylib-dev` (ou compilar a raylib a partir do código-fonte) |
| Windows | MinGW + raylib; se preciso, ajuste `RAYLIB_PATH` no `Makefile` |

Na pasta do projeto:

```bash
make          # compila (gera o executável pawpocalypse)
make run      # compila e já abre o jogo
make clean    # apaga o que a compilação gerou (pasta build/ e o executável)
```

O jogo abre numa janela de 1280x720 que pode ser redimensionada (F11 alterna a tela cheia). Tudo é desenhado numa tela interna de 640x360 e ampliado em escala inteira, sem suavização, para manter o visual pixelado.

---

## 2. Como jogar

Os gatos zumbis vêm pela direita, linha por linha, rumo à casa à esquerda. Coloque cachorros no campo (4 linhas x 8 colunas) para detê-los. **Se um gato chegar à casa, a fase está perdida.**

| Ação | Como fazer |
|---|---|
| Escolher o deck | Antes de cada fase, escolha 5 das 10 raças (o Pote de Ração vem sempre junto) |
| Colocar um cachorro | Clique na carta (ou teclas **1** a **6**) e depois numa célula livre |
| Ganhar petiscos | Clique nos petiscos que caem do céu ou saem do Pote de Ração (cada um vale 25) |
| Tirar um cachorro | Pá: botão ou tecla **S** |
| Mudar um cachorro de lugar | Luva: botão ou tecla **L** (só 2 vezes por fase) |
| Acelerar | Botão **2x** ou tecla **V** |
| Cancelar a seleção | Botão direito do mouse |
| Pausar | **ESC** ou **P** |
| Música | **M** liga/desliga; os volumes da música e dos efeitos ficam no canto do menu |

Cada cachorro custa petiscos e tem um tempo de recarga. O jogador começa cada fase com **200 petiscos**. Na **fase 5**, a do chefão, não há petiscos: os cachorros chegam **de graça por uma esteira** à esquerda.

Ao final de cada fase aparece a **tela de resultado** com o tempo e a pontuação. As melhores vitórias entram no **Ranking (Top 10)**, com o nome do jogador.

---

## 3. Conteúdo do jogo

### Fases

| Fase | Como é |
|---|---|
| Tutorial | Ensina passo a passo: coletar petiscos, comprar o pote, colocar um cachorro, usar a pá e enfrentar um gato |
| 1 a 4 | 4 hordas cada (1 fácil, 2 intermediárias, 1 difícil); cada fase traz gatos novos |
| 5 (chefão) | O **Rei dos Gatos Zumbis** ocupa a última coluna, invoca gatos e lança bolas de pelo; suas 4 hordas mudam a cada ¼ de vida perdida |

Antes de cada horda, um anúncio com sirene avisa que os gatos estão chegando.

### As 10 raças (e o Pote de Ração)

| Raça | Papel | Custo | O que faz |
|---|---|---|---|
| Pote de Ração | Gera petiscos | 50 | Solta um petisco a cada 15 s |
| Beagle | Atirador | 100 | Cospe uma gota de baba (20 de dano) a cada 1,5 s |
| Bulldog | Muralha | 50 | Aguenta 4000 de dano; o Gato de Telhado não consegue pulá-lo |
| Dálmata | Atirador duplo | 200 | Cospe duas bolhas de baba (20 cada) a cada 1,5 s |
| Husky | Latido gelado | 175 | Cospe baba congelada (20) que deixa o gato lento |
| Pastor Alemão | Mordida forte | 150 | Late e morde (40) todos os gatos até 1,5 célula à frente |
| Westie | Toca-armadilha | 25 | Cava uma toca por 10 s; depois salta no 1º gato que chegar (600) |
| São Bernardo | Cura vizinhos | 125 | Devolve 50 de vida aos 8 vizinhos a cada 5 s |
| Dachshund | Ataca 3 linhas | 300 | Cospe um jato de baba (20) na sua linha e nas duas vizinhas |
| Rottweiler | Explosão 3x3 | 150 | Explode em 1 s e causa 1800 na área 3x3 (fere também o Rei) |
| Shiba Inu | Grito assustador | 125 | O "grito Shiba" faz os gatos à frente recuarem meia célula |

### Os gatos zumbis

| Gato | Vida | Pontos | Característica |
|---|---|---|---|
| Gato Zumbi | 200 | 10 | O mais fraco |
| Gato da Caixa | 560 | 25 | Protegido por uma caixa de leite |
| Gato da Lata | 1300 | 50 | Protegido por uma lata |
| Siamês | 180 | 20 | Rápido |
| Persa | 450 | 35 | Pelo fofo: imune ao gelo |
| Gato de Telhado | 250 | 30 | Pula o 1º cachorro (menos o Bulldog) |
| Maine Coon | 3000 | 100 | Tanque: enorme e com mordida forte |

### Pontuação

`pontos dos gatos derrotados` + (só na vitória) `500 × número da fase` (+2000 pelo Rei) + `bônus de tempo` (1000 menos 1 ponto por segundo de partida).

O save guarda o **recorde de cada fase**, e os pontos do save são a soma desses recordes. Assim, repetir uma fase não infla a pontuação.

---

## 4. Onde está cada requisito do trabalho

| Requisito | Onde está no código |
|---|---|
| **Structs** | `Partida` (partida.h), `Mapa` (map.h), `Cachorro`/`InfoCachorro` (tower.h), `Gato`/`InfoGato` (enemy.h), `Hordas` (horda.h), `Chefao` (chefao.h), `Esteira` (esteira.h), `Save` (save.h), `Recorde`/`Ranking` (highscore.h), `Jogo` (game.h) |
| **Ponteiros** | O campo é uma matriz de **ponteiros** para cachorros (`Cachorro *grade[4][8]`); as listas são percorridas com **ponteiro para ponteiro** (`remover_no` em enemy.c, projetil.c e petisco.c) |
| **Ponteiros de função** | Tabela `habilidades[]` em habilidade.c: cada raça aponta para a função da sua habilidade |
| **malloc / free** | `jogo_criar`, `partida_criar`/`partida_destruir`, `cachorro_criar`/`cachorro_destruir`, `gato_adicionar`, `projetil_adicionar`, `petisco_adicionar`, `chefao_criar`/`chefao_destruir`; a música é gerada num vetor alocado (audio.c) |
| **Listas encadeadas** | Gatos em campo (enemy.c), babas em voo (projetil.c) e petiscos (petisco.c) |
| **Matrizes** | Campo 4x8 (`Mapa.grade`), hordas `[fase][horda][tipo de gato]` (horda.c), acordes da música (audio.c) e as figuras em pixel-art guardadas como matrizes de caracteres (petisco.c, projetil.c, chefao.c) |
| **Escrita e leitura de arquivo** | Saves em `assets/data/saves.txt` (save.c) e ranking em `assets/data/highscores.txt` (highscore.c), com `fopen`, `fgets`, `sscanf` e `fprintf` |
| **Makefile** | `Makefile` na raiz (`make`, `make run`, `make clean`), com regras para macOS, Linux e Windows |
| **Tela inicial com botões** | Menu em game.c: Jogar, Tutorial, Instruções, Ranking e Sair |
| **5 fases + tutorial, chefão na 5ª** | Fases 1–4 em horda.c; tutorial em partida.c; chefão em chefao.c e esteira.c |
| **Mapa 4x8 e derrota se um gato chegar à casa** | map.h (`LINHAS_MAPA` 4, `COLUNAS_MAPA` 8); `gato_atualizar_lista` avisa quando um gato passa da casa e a partida vira derrota |
| **Escolher 5 de 10 cachorros** | Tela do deck (deck.c) |
| **Petiscos como moeda, começando com 200** | `PETISCOS_INICIAIS` (partida.h) |
| **Tela de resultado com pontuação e tempo** | `desenhar_resultado` em game.c e `partida_pontuacao` em partida.c |
| **4 hordas por fase (1 fácil, 2 intermediárias, 1 difícil)** | Tabela de horda.c; na fase 5, a tabela `hordas[]` de chefao.c |
| **Visual pixelado** | Tela interna de 640x360 ampliada com filtro POINT (game.c) e sprites de 64x64 (sprite.c) |
| **GitHub** | Uma branch por etapa, com merge na `main` (veja o histórico do repositório) |

---

## 5. Mapa do código

O programa é dividido em **módulos**: cada um tem um `.h` em `include/`, com as structs e os protótipos comentados, e um `.c` em `src/`.

```
main.c
  └─ game.c ─────────── telas: menu, saves, fases, deck, instruções, ranking, partida, pausa, resultado
       ├─ deck.c          tela de escolha das 5 raças
       ├─ save.c          os 3 saves (arquivo)
       ├─ highscore.c     o Top 10 (arquivo)
       ├─ audio.c         música de fundo (composta no próprio código)
       ├─ som.c           efeitos sonoros (WAVs de assets/sons)
       └─ partida.c ────── uma fase em andamento: cartas, petiscos, pá, luva, 2x, tutorial, HUD
            ├─ map.c        o campo 4x8 (matriz de ponteiros para cachorros)
            ├─ tower.c      tabela das raças e desenho dos cachorros
            ├─ habilidade.c o que cada raça faz (tabela de ponteiros de função)
            ├─ enemy.c      tabela dos gatos e a lista encadeada dos gatos
            ├─ projetil.c   lista encadeada das babas
            ├─ petisco.c    lista encadeada dos petiscos
            ├─ horda.c      as 4 hordas das fases 1 a 4
            ├─ chefao.c     o Rei dos Gatos (fase 5)
            └─ esteira.c    as cartas grátis da fase 5
Usados por todos: ui.c (botões, textos, painéis, deslizadores, pixel-art),
sprite.c (spritesheets), cenario.c (céu e prédios), config.h (constantes).
```

| Módulo | Responsabilidade |
|---|---|
| `main` | Abre a janela e roda o laço principal (atualizar → desenhar) |
| `game` | Máquina de estados das telas; cria e encerra as partidas; grava saves e ranking |
| `partida` | Uma partida: atualiza cachorros, gatos, hordas, babas e petiscos; trata a entrada do jogador; calcula a pontuação; desenha a fase |
| `map` | O campo 4x8: colocar, mover, remover e desenhar cachorros |
| `tower` | Tabela das raças (custo, vida, dano, sprite...) e o desenho de cada uma |
| `habilidade` | Uma função por habilidade e a tabela de ponteiros de função que liga cada raça à sua |
| `enemy` | Tabela dos gatos e a lista encadeada dos gatos: andar, morder, pular, morrer |
| `projetil` | Lista encadeada das babas: voar, acertar gatos e chegar ao Rei |
| `petisco` | Lista encadeada dos petiscos: cair, ser coletado, sumir |
| `horda` | As 4 hordas de cada fase (matriz `[fase][horda][tipo]`) e o envio dos gatos |
| `chefao` | O Rei dos Gatos: vida, hordas, invocações e bola de pelo |
| `esteira` | As cartas grátis que sobem pela esteira na fase 5 |
| `deck` | A tela de escolha das 5 raças |
| `save` | Leitura e escrita dos 3 saves, com o recorde de cada fase |
| `highscore` | Leitura, escrita, inserção ordenada e desenho do Top 10 |
| `sprite` | Tabela dos spritesheets, animação por tempo e retratos para as cartas |
| `som` | Tabela dos efeitos sonoros e o volume dos efeitos |
| `audio` | Música chiptune gerada em memória e o volume da música |
| `ui` | Botões, textos, painéis, barras de vida, deslizadores e desenho de pixel-art |
| `cenario` | Céu do pôr do sol, prédios e o título animado |

**Um frame de partida, em ordem:** os cachorros agem (habilidades) → chegam gatos (hordas ou o Rei) → as babas voam e acertam → os gatos andam ou mordem (e os derrotados saem da lista) → confere vitória ou derrota → os petiscos caem → trata os cliques do jogador.

---

## 6. Pastas do projeto

```
Makefile           compilação
README.md          este arquivo
FUNCOES.md         todas as funções, com o que cada uma faz
include/           os .h (structs e protótipos comentados)
src/               os .c (uma função por bloco comentado)
assets/
  sprites/         spritesheets 64x64 dos cachorros, gatos e pote (+ LEIAME e FALTANDO.txt)
  sons/            efeitos sonoros .wav (+ LEIAME e o gerador gerar_sons.py)
  data/            saves.txt (criado ao jogar) e highscores.txt (o Top 10)
```

Os sprites e os sons podem ser trocados por versões melhores mantendo os mesmos nomes de arquivo, sem mudar o código. Se um arquivo faltar, o jogo usa formas geométricas (sprites) ou fica sem aquele efeito (sons).

---

## 7. Teclas de teste

Esta versão tem dois **atalhos de desenvolvimento**, para testar as fases rapidamente (eles aparecem como lembrete na tela de fases):

| Tecla | Efeito |
|---|---|
| `]` | +100 petiscos |
| `\` | vence a fase na hora |

Estão todos na função `tratar_atalhos_teste` (partida.c). Para a versão entregue ao jogador, basta apagá-la (e a sua chamada em `tratar_entrada`) e tirar a dica da tela de fases (`desenhar_fases`, em game.c).

---

## 8. Lista de todas as funções

O arquivo **[FUNCOES.md](FUNCOES.md)** lista as **238 funções** do jogo, arquivo por arquivo, cada uma com uma frase sobre o que faz:
- **116 públicas**, declaradas nos `.h` (incluindo a `main`);
- **122 internas**, `static` nos `.c`.

No código, cada função tem um bloco de comentário logo acima explicando o que faz, e cada protótipo nos `.h` tem um resumo de uma linha ao lado.
