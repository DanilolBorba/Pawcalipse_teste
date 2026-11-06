# PawPocalypse — Todas as funções

Lista de **todas as funções do jogo**, arquivo por arquivo, na ordem em que
aparecem no código. A descrição é a primeira frase do comentário que fica
acima de cada função (o comentário completo está no `.c`).

- **pública**: declarada no `.h` do módulo e usada por outros arquivos.
- **interna**: `static`, usada só dentro do próprio `.c`.

Para a visão geral do código, veja o [README](README.md).

## `src/main.c`

Ponto de entrada do programa.

| Função | Tipo | O que faz |
|---|---|---|
| `main` | pública | inicia o gerador aleatório, abre a janela (redimensionável, 60 FPS), desliga o "ESC fecha o jogo" da raylib (aqui o ESC pausa), muda a pasta de trabalho para a do executável (para achar assets/) e roda o loop principal até o jogador sair ou fechar a janela. |

## `src/game.c`

Máquina de estados das telas: menu, saves, fases, instruções, ranking, partida (com a tela de resultado) e pausa.

| Função | Tipo | O que faz |
|---|---|---|
| `area_da_tela` | interna | calcula onde a tela virtual aparece dentro da janela. |
| `texto_tempo` | interna | formata segundos como "mm:ss". |
| `gravar_saves` | interna | grava os saves no arquivo e avisa no terminal se falhar. |
| `iniciar_partida` | interna | cria a partida da fase pedida (FASE_TUTORIAL para o tutorial) com o deck escolhido, toca o som de início e vai para a tela de jogo. |
| `abrir_fase` | interna | as fases passam antes pela escolha do deck; o tutorial, que tem deck fixo, e a fase do chefão, que usa a esteira, começam direto. |
| `atualizar_deck` | interna | na escolha do deck, começa a fase ou volta ao mapa. |
| `lugar_no_ranking` | interna | lugar (0 a 9) que a partida terminada ocupa no Top 10, ou -1 se ela não entra (só as vitórias nas fases entram, o tutorial não). |
| `encerrar_partida` | interna | soma o tempo jogado ao save atual e, se a partida foi vencida, marca a fase como concluída, libera a próxima e guarda os pontos se forem o recorde da fase; grava o arquivo. |
| `atualizar_menu` | interna | troca de tela conforme o botão clicado no menu e trata os deslizadores de volume da música e dos efeitos (ao soltar o dos efeitos, toca um som para o jogador ouvir o volume novo). |
| `desenhar_menu` | interna | cenário, título balançando, subtítulo, botões, versão e, no canto, os deslizadores de volume (ou "Sem áudio"). |
| `retangulo_save` | interna | área do cartão do save i. |
| `botoes_do_save` | interna | monta os botões do cartão do save i conforme a situação: espaço vazio (Novo Jogo), save em uso (Jogar / Apagar) ou confirmação de exclusão (Sim / Não). |
| `atualizar_saves` | interna | Voltar/ESC retorna ao menu. |
| `desenhar_saves` | interna | os 3 cartões de save. |
| `posicao_fase` | interna | centro do círculo da fase f (0 a 4) no mapa de fases. |
| `atualizar_fases` | interna | Voltar/ESC retorna aos saves; clicar numa fase liberada abre a escolha do deck dela. |
| `desenhar_fases` | interna | as 5 fases do save como círculos ligados por uma trilha: verde = vencida, marrom = liberada, cinza = bloqueada; a 5ª (chefão) tem borda vermelha. |
| `desenhar_ranking` | interna | painel com o Top 10 (lugar, nome, fase, pontos e tempo de cada recorde) e o botão Voltar. |
| `desenhar_instrucoes` | interna | painel com o texto de como jogar e o botão Voltar. |
| `tem_proxima_fase` | interna | true se, depois desta partida, o 1º botão leva para a fase seguinte (vitória numa fase que não é a última nem o tutorial). |
| `botao_fim_partida` | interna | monta o 1º botão da tela de fim de partida: "Próxima Fase", "Jogar de Novo" (vitória na última fase ou no tutorial) ou "Tentar de Novo" (derrota). |
| `botao_voltar_fim` | interna | o 2º botão da tela de fim de partida, que volta ao mapa de fases (ou ao menu, se era o tutorial). |
| `digitar_nome` | interna | lê as letras digitadas (caracteres comuns, até TAMANHO_NOME) e o Backspace, que apaga a última. |
| `atualizar_fim_partida` | interna | na tela de vitória/derrota, se a vitória entrar no Top 10, o jogador digita o nome. |
| `atualizar_jogando` | interna | com o resultado decidido, trata a tela de vitória ou derrota. |
| `desenhar_linha_pontos` | interna | uma linha da conta de pontos: o texto à esquerda e o valor ("+N") alinhado à direita. |
| `desenhar_resultado` | interna | abaixo do título, a conta de pontos da partida: gatos derrotados, bônus de vitória e de tempo e o total. |
| `desenhar_jogando` | interna | desenha a partida; com o resultado decidido, escurece a tela e mostra por cima o título (vitória em verde, derrota em vermelho), o tempo da partida, a conta de pontos (fora do tutorial) e os dois botões. |
| `desenhar_pausa` | interna | a partida congelada ao fundo, escurecida, com os botões Continuar e Sair da Partida por cima. |
| `jogo_criar` | pública | aloca o jogo com malloc, cria a tela virtual com filtro POINT (ao ampliar, cada pixel vira um bloco, sem borrar), carrega os sprites, lê os saves do arquivo, começa a música e carrega os efeitos sonoros. |
| `jogo_atualizar` | pública | avança o tempo e a música, converte o mouse para a tela virtual (SetMouseOffset/SetMouseScale desfazem a escala e as bordas), trata o F11 e executa a lógica da tela atual. |
| `jogo_desenhar` | pública | desenha a tela atual na tela virtual e depois a amplia para a janela. |
| `jogo_destruir` | pública | se o jogo for fechado durante uma partida, salva o tempo jogado (e a vitória, se já estava na tela de vitória); depois libera os efeitos sonoros, a música, os sprites, a tela virtual e o próprio jogo. |

## `src/partida.c`

Uma partida: cartas, petiscos, colocação de cachorros, pá, luva, velocidade 2x, combate contra os gatos, tutorial e a fase do chefão (com a esteira).

| Função | Tipo | O que faz |
|---|---|---|
| `retangulo_carta` | interna | área da carta de índice i na barra superior. |
| `avisar` | interna | mostra uma mensagem temporária abaixo da barra superior. |
| `avancar_tutorial` | interna | se a partida for o tutorial e o jogador acabou de cumprir o passo atual, passa para a próxima instrução. |
| `montar_deck` | interna | monta as cartas da partida: o Pote de Ração e, depois dele, os cachorros do deck, todos prontos para comprar. |
| `partida_criar` | pública | aloca a partida com malloc, zera tudo e prepara o campo vazio, as listas de petiscos, gatos e babas vazias, o saldo inicial, a luva com todos os usos, a esteira vazia e as cartas: no tutorial, um deck fixo; na fase do chefão, nenhuma (os cachorros vêm pela esteira) e o Rei, alocado com malloc; nas outras fases, o deck escolhido pelo jogador e as hordas. |
| `partida_destruir` | pública | libera os cachorros do campo, as listas de petiscos, gatos e babas, o chefão (se houver) e a própria partida. |
| `partida_eh_tutorial` | pública | retorna true se a partida for o tutorial. |
| `soltar_petisco_do_ceu` | interna | de tempos em tempos (8 a 12 s) solta um petisco do céu num ponto aleatório do campo. |
| `agir_cachorro` | interna | avança a animação do cachorro e conta o tempo até a próxima ação dele; quando ele chega a zero, usa a habilidade da raça e, se ela agiu, recomeça a contar a cadência. |
| `agir_cachorros` | interna | percorre a matriz do mapa fazendo cada cachorro agir e remove os que ficaram sem vida (as minas e o Rottweiler, que explodem). |
| `ocupar_vaga_efeito` | interna | guarda o efeito na 1ª vaga livre do vetor de efeitos. |
| `partida_adicionar_efeito` | pública | mostra um círculo (explosão, cura...) que cresce até 'raio' e some. |
| `partida_adicionar_latido` | pública | mostra (e toca) o latido do cachorro da célula 'centro': ondas que se abrem até 'alcance' e o texto, na cor pedida. |
| `desenhar_latido` | interna | três ondas (arcos) saindo da boca do cachorro para a direita, abrindo até o alcance, e o texto do latido subindo acima da cabeça. |
| `desenhar_efeitos` | interna | cada efeito ativo é um latido ou um círculo que cresce até o raio dele enquanto vai ficando transparente. |
| `cancelar_selecao` | interna | desmarca a carta (da barra ou da esteira), a pá e a luva (soltando o cachorro que ela tiver pegado, sem gastar um uso). |
| `selecionar_carta` | interna | marca a carta i para ser colocada no campo (desmarcando a pá e a luva). |
| `selecionar_da_esteira` | interna | marca a carta i da esteira para ser colocada no campo (desmarcando o resto). |
| `tipo_selecionado` | interna | raça da carta selecionada (na barra ou na esteira), ou -1 se nenhuma estiver. |
| `celula_do_chefao` | interna | true se a coluna for o território do Rei (só na fase do chefão), onde não se pode pôr nem mover cachorros. |
| `alternar_pa` | interna | pega a pá (desmarcando o resto) ou a larga, se já estava com ela. |
| `alternar_luva` | interna | pega a luva (desmarcando o resto) ou a larga, se já estava com ela. |
| `cachorro_na_luva` | interna | retorna o cachorro pego pela luva, ou NULL se ela estiver vazia (ou se ele morreu enquanto estava pego). |
| `usar_luva` | interna | com a luva vazia, clicar num cachorro o pega (ele só fica marcado). |
| `colocar_cachorro` | interna | cria o cachorro da carta selecionada (malloc) e o guarda na célula, se ela estiver livre. |
| `tratar_atalhos_teste` | interna | ATALHOS DE TESTE, só para desenvolvimento (tirar na versão entregue ao jogador), lidos pelo caractere digitado para funcionarem em qualquer teclado: ']' dá PETISCOS_TESTE petiscos e '\' vence a fase na hora. |
| `tratar_teclas` | interna | atalhos do teclado: 1 a 6 escolhem cartas, S pega a pá, L pega a luva, V liga/desliga o 2x e o botão direito cancela a seleção. |
| `clicar_na_barra` | interna | trata um clique nos botões da barra superior (cartas, pá, luva e 2x). |
| `clicar_no_campo` | interna | age na célula clicada conforme o que está selecionado: remove o cachorro com a pá, move com a luva ou coloca o da carta. |
| `tratar_entrada` | interna | lê os atalhos (de teste e do jogo) e o clique esquerdo, nesta ordem de prioridade: coletar petisco, botões da barra superior, cartas da esteira e, por fim, a célula do campo. |
| `fase_vencida` | interna | true quando as 4 hordas da fase foram vencidas; na fase do chefão, quando o Rei foi derrotado; no tutorial, quando o gato do último passo foi derrotado. |
| `anuncio_atual` | interna | o texto do anúncio da horda que está na tela (das hordas ou, na fase do chefão, do Rei), ou NULL se não houver nenhum. |
| `partida_atualizar` | pública | avança o tempo da partida, o aviso, os efeitos e a recarga das cartas; faz os cachorros agirem; na fase do chefão, faz a esteira andar e o Rei agir (e leva até ele o dano das babas que chegam à sua frente); nas outras, manda os gatos das hordas (fora do tutorial) e solta petiscos do céu. |
| `partida_pontuacao` | pública | soma os gatos derrotados de cada tipo e os pontos que cada tipo vale (tabela dos gatos). |
| `desenhar_carta` | interna | desenha a carta i: fundo, retrato do cachorro, custo (em vermelho se não der para comprar), sombra de recarga que encolhe com o tempo e borda verde se estiver selecionada. |
| `desenhar_botao` | interna | fundo e borda de um botão da barra superior (pá, luva, 2x): verde e com borda clara enquanto estiver ativo. |
| `desenhar_ferramentas` | interna | os botões da pá e da luva (ainda sem ícone: o nome e a tecla). |
| `desenhar_botao_acelerar` | interna | o botão do 2x, com o ícone de avançar (dois triângulos apontando para a direita); fica verde com o jogo acelerado. |
| `desenhar_info` | interna | área entre a luva e o 2x: a fase, o tempo e, fora do tutorial, "Horda N/4" com a barra que enche conforme os gatos da fase vão saindo (na fase do chefão, conforme o Rei perde vida); por último, o lembrete do ESC. |
| `desenhar_hud` | interna | barra superior com o contador de petiscos e as cartas (na fase do chefão, a vida do Rei no lugar deles), a pá, a luva, as informações da partida e o botão do 2x. |
| `desenhar_dica_carta` | interna | com o mouse sobre uma carta, mostra logo abaixo dela uma caixinha com o nome e o papel do cachorro. |
| `desenhar_previa` | interna | marca de laranja a célula do cachorro pego pela luva e destaca a célula sob o mouse. |
| `desenhar_mensagem` | interna | mostra entre a barra e o campo o aviso temporário (em vermelho) ou, no tutorial, a instrução do passo atual com um contorno laranja em volta do que deve ser clicado. |
| `desenhar_anuncio_horda` | interna | pouco antes de cada horda (na fase do chefão, quando ela começa), cobre o meio do campo com uma faixa vermelho-escura de bordas vermelhas e escreve o anúncio em letras grandes, piscando entre vermelho e branco, para o jogador não deixar passar. |
| `partida_desenhar` | pública | desenha, de trás para a frente, o céu e os prédios atrás da barra, o campo com os cachorros, o Rei (na fase do chefão), os gatos, as babas, as explosões, os latidos e curas, a prévia sob o mouse, a barra superior, a esteira (só na fase do chefão), a faixa de mensagens, a dica da carta sob o mouse, o anúncio da horda e os petiscos (por cima de tudo). |

## `src/map.c`

Campo de batalha (matriz 4x8), a casa e a rua.

| Função | Tipo | O que faz |
|---|---|---|
| `mapa_iniciar` | pública | marca todas as células da matriz como vazias (NULL). |
| `mapa_limpar` | pública | percorre a matriz removendo (e liberando) todos os cachorros. |
| `mapa_celula_no_ponto` | pública | converte um ponto da tela na linha e na coluna da célula, devolvidas pelos ponteiros. |
| `mapa_retangulo_celula` | pública | retorna a área (x, y, largura, altura) da célula. |
| `mapa_centro_celula` | pública | retorna o ponto central da célula. |
| `mapa_colocar` | pública | guarda o ponteiro do cachorro na célula indicada pelos campos linha/coluna dele. |
| `mapa_remover` | pública | libera o cachorro da célula (free de NULL é seguro, então funciona mesmo com a célula vazia) e marca a célula como livre. |
| `mapa_mover` | pública | passa o ponteiro do cachorro da célula atual para a célula de destino e atualiza a posição guardada nele (vida e timer continuam os mesmos). |
| `desenhar_cachorro_no_campo` | interna | desenha o cachorro no centro da sua célula, no quadro atual da animação (o Westie fica escuro enquanto cava a toca) e, se ele estiver ferido, a barra de vida logo acima. |
| `mapa_desenhar` | pública | desenha o gramado em xadrez, a casa à esquerda (telhado, parede, janela acesa, porta e a placa "LAR"), a rua à direita, a terra embaixo e, por fim, cada cachorro da matriz no centro da sua célula. |

## `src/tower.c`

Tabela das raças e o desenho dos cachorros e do pote (sprites, com formas geométricas como reserva).

| Função | Tipo | O que faz |
|---|---|---|
| `cachorro_info` | pública | retorna um ponteiro para os dados fixos do tipo na tabela. |
| `cachorro_criar` | pública | aloca um cachorro com malloc, com a vida cheia do tipo e a posição na matriz. |
| `cachorro_destruir` | pública | libera a memória do cachorro (aceita NULL). |
| `cachorro_quadro` | pública | retorna o quadro da animação do cachorro no campo. |
| `cachorro_desenhar` | pública | desenha o tipo centrado em 'centro', com tamanho 'raio' (RAIO_CACHORRO = tamanho no campo, escala 1 do sprite). |
| `cachorro_desenhar_retrato` | pública | nas cartas, desenha só a cabeça do cachorro (a tigela, no pote) em tamanho real, centrada em 'centro'. |

## `src/habilidade.c`

As habilidades das raças e a tabela de ponteiros de função que liga cada raça à sua.

| Função | Tipo | O que faz |
|---|---|---|
| `centro_do` | interna | centro da célula onde o cachorro está. |
| `tem_alvo` | interna | true se há um gato na linha à frente do ponto x ou se é a fase do chefão (aí sempre vale atirar: as babas que passam acertam o Rei). |
| `cuspir` | interna | cria uma baba do estilo pedido com o dano do cachorro, saindo da boca dele (no sprite) mas na linha pedida; 'recuo' a põe alguns pixels mais para trás. |
| `produzir_petisco` | interna | o Pote de Ração solta um petisco perto de onde aparece a estrela do sprite, caindo à direita da tigela (para não se esconder no monte de ração do desenho), com o barulho da ração caindo. |
| `atirar` | interna | o Beagle cospe uma gota de baba, se houver um alvo na linha à frente. |
| `atirar_duplo` | interna | o Dálmata, se houver um alvo na linha à frente, cospe duas bolhas de baba seguidas (a segunda um pouco mais atrás). |
| `atirar_gelo` | interna | o Husky cospe uma baba congelada, que deixa o gato lento. |
| `atirar_tres_linhas` | interna | o Dachshund, se houver alvo na sua linha ou numa vizinha, cospe um jato de baba em cada uma dessas três linhas (as que existem). |
| `morder` | interna | o Pastor Alemão late ("AU!") e morde de uma vez todos os gatos da sua linha que estejam até 1,5 célula à frente. |
| `atacar_da_toca` | interna | o Westie, um terrier, passa os primeiros segundos cavando uma toca (só depois disso esta função é chamada). |
| `gritar` | interna | o Shiba Inu solta o famoso "grito Shiba" (um latido longo e amarelo) quando há gato na sua linha até 2,5 células à frente: o susto faz esses gatos recuarem meia célula. |
| `explodir_area` | interna | o Rottweiler explode, ferindo os gatos da área 3x3 em volta dele (a linha dele e as duas vizinhas) e o Rei, se a área chegar até ele, e some. |
| `curar_vizinhos` | interna | o São Bernardo devolve vida aos cachorros das 8 células em volta dele, sem passar da vida máxima de cada um. |
| `habilidade_usar` | pública | busca na tabela o ponteiro da função da raça e a chama. |

## `src/enemy.c`

Tabela dos gatos e a lista encadeada dos gatos em campo.

| Função | Tipo | O que faz |
|---|---|---|
| `gato_info` | pública | retorna um ponteiro para os dados fixos do tipo na tabela. |
| `gato_adicionar` | pública | aloca um gato com malloc, com a vida cheia do tipo, logo depois da borda direita da tela (na rua), o insere no INÍCIO da lista e toca o gemido de zumbi. |
| `remover_no` | interna | tira da lista o nó apontado por 'ligacao' (a cabeça ou o campo 'proximo' do nó anterior), religa a lista ao nó seguinte e libera o nó. |
| `cachorro_a_frente` | interna | retorna o cachorro da célula onde fica a "boca" do gato (um pouco à esquerda do centro), ou NULL se o caminho estiver livre. |
| `mover_ou_morder` | interna | com o caminho livre, o gato anda para a esquerda e avança a animação no mesmo ritmo (para as patas não deslizarem). |
| `gato_atualizar_lista` | pública | percorre a lista com um ponteiro para ponteiro: remove os gatos sem vida (somando 1 em derrotados[tipo], o vetor com a contagem de cada tipo), conta os tempos de dano e de lentidão e faz os outros andarem ou morderem. |
| `gato_na_mira` | pública | retorna true se existe um gato vivo na linha, à direita do ponto x e já dentro da tela (usado pelos atiradores para decidir se atiram). |
| `gato_atingido` | pública | procura um gato vivo na linha cujo corpo cubra o ponto x. |
| `gato_empurrar_area` | pública | faz todos os gatos vivos da linha cujo centro esteja entre xMin e xMax recuarem 'distancia' pixels para a direita (o susto do grito do Shiba). |
| `gato_ferir_area` | pública | fere com 'dano' todos os gatos vivos da linha cujo centro esteja entre xMin e xMax (mordidas e explosões). |
| `gato_ferir` | pública | tira 'dano' da vida do gato e o deixa avermelhado por um instante. |
| `gato_gelar` | pública | deixa o gato lento por DURACAO_LENTIDAO segundos, a não ser que ele seja imune ao gelo (Persa). |
| `tinta_do_gato` | interna | cor multiplicada no desenho: avermelhada logo depois de levar dano, azulada enquanto estiver lento e, senão, a tinta do tipo. |
| `desenhar_gato_formas` | interna | desenho reserva, usado se o PNG não existir, com os pés em 'pes': rabo, corpo, orelhas, cabeça, olhos vermelhos e, se ainda tiver, a caixa (branca) ou a lata (cinza) na cabeça. |
| `desenhar_gato` | interna | desenha o gato com os pés perto da base da sua linha. |
| `gato_desenhar_lista` | pública | percorre a lista desenhando cada gato. |
| `gato_liberar_lista` | pública | remove a cabeça da lista até ela ficar vazia (NULL), liberando todos os nós. |

## `src/horda.c`

Tabela das hordas e o envio dos gatos de cada horda.

| Função | Tipo | O que faz |
|---|---|---|
| `carregar_horda` | interna | copia da tabela quantos gatos de cada tipo a horda atual vai mandar e zera o timer, para o 1º gato sair assim que a pausa acabar. |
| `horda_iniciar` | pública | zera o controle, soma quantos gatos a fase tem no total (para a barra de progresso), carrega a 1ª horda e começa a preparação. |
| `gatos_faltando` | interna | quantos gatos da horda atual ainda não saíram. |
| `sortear_tipo` | interna | sorteia o tipo do próximo gato entre os que faltam, com chance proporcional à quantidade de cada tipo, e o desconta da horda. |
| `horda_atualizar` | pública | durante a pausa, só conta o tempo. |
| `horda_terminou` | pública | retorna true quando as 4 hordas já foram vencidas. |
| `horda_numero` | pública | número da horda atual, de 1 a 4 (para mostrar na tela). |
| `horda_progresso` | pública | fração dos gatos da fase que já saíram, de 0 a 1. |
| `horda_anuncio` | pública | nos últimos segundos de cada pausa, retorna o texto que anuncia a horda que vem; fora disso, retorna NULL. |

## `src/chefao.c`

O Rei dos Gatos Zumbis: hordas, invocações, bola de pelo e desenho.

| Função | Tipo | O que faz |
|---|---|---|
| `chefao_criar` | pública | aloca o Rei com malloc, com a vida cheia, na 1ª horda e sem bola rolando; o anúncio da chegada dele já começa na tela. |
| `chefao_destruir` | pública | libera a memória do chefão (aceita NULL). |
| `sortear_gato` | interna | sorteia o tipo de um gato invocado, com chance proporcional ao peso de cada tipo na horda atual. |
| `invocar_gatos` | interna | põe na lista os gatos de uma invocação da horda atual, cada um numa linha aleatória, saindo da frente do Rei (um pouco espaçados, para não ficarem um em cima do outro). |
| `trocar_horda` | interna | a horda vem da vida já perdida (uma nova a cada quarto). |
| `sortear_linha_com_cachorro` | interna | sorteia uma das linhas que têm pelo menos um cachorro. |
| `rolar_bola` | interna | a bola de pelo rola para a esquerda; ao entrar na célula de um cachorro (o mais à direita da linha), o derruba e some. |
| `chefao_atualizar` | pública | conta os tempos do Rei, troca de horda conforme a vida, invoca gatos a cada intervalo da horda e, a cada INTERVALO_BOLA segundos, rola uma bola de pelo numa linha que tenha cachorros. |
| `chefao_ferir` | pública | tira 'dano' da vida (sem passar de zero) e deixa o Rei avermelhado por um instante. |
| `chefao_derrotado` | pública | retorna true quando a vida do Rei acabou. |
| `chefao_fracao_vida` | pública | quanto da vida ainda resta, de 0 (derrotado) a 1. |
| `chefao_anuncio` | pública | enquanto o anúncio estiver na tela, retorna o texto da horda atual (a chegada do Rei, a 2ª, a 3ª ou a última); senão, NULL. |
| `desenhar_bola` | interna | a bola de pelo, com três tufos que giram enquanto ela rola. |
| `chefao_desenhar` | pública | escurece de roxo a última coluna e a rua (o território do Rei) e desenha o Rei: o sprite do gato zumbi ampliado e arroxeado (avermelhado logo depois de levar dano), subindo e descendo devagar como se respirasse, com a coroa em pixel-art na cabeça. |
| `chefao_desenhar_vida` | pública | na área da barra superior, o nome do Rei e a barra de vida, dividida em 4 partes (uma por horda), com a vida em números. |

## `src/esteira.c`

A esteira de cartas grátis da fase do chefão.

| Função | Tipo | O que faz |
|---|---|---|
| `sortear_raca` | interna | sorteia a raça da próxima carta, com chance proporcional ao peso de cada uma na tabela. |
| `retangulo_carta_esteira` | interna | área da carta i, na altura em que ela está agora. |
| `esteira_iniciar` | pública | deixa a esteira vazia, sem carta selecionada e com o timer zerado, para a 1ª carta chegar logo. |
| `esteira_atualizar` | pública | faz cada carta subir até a sua vaga (a vaga i fica logo abaixo da i-1). |
| `esteira_carta_no_ponto` | pública | retorna o índice da carta que contém o ponto, ou -1 se ele não estiver sobre nenhuma. |
| `esteira_remover` | pública | tira a carta i do vetor, puxando as seguintes uma casa para cima (elas sobem até a nova vaga na animação), e desmarca a seleção. |
| `desenhar_dica_esteira` | interna | com o mouse sobre uma carta, mostra à direita dela uma caixinha com o nome e o papel do cachorro. |
| `esteira_desenhar` | pública | a faixa escura com listras que sobem (dando a ideia de movimento), as cartas com o retrato do cachorro (sem custo: são de graça), com borda verde na selecionada e laranja sob o mouse, e a dica da carta sob o mouse. |

## `src/projetil.c`

Lista encadeada das babas em voo e o desenho de cada estilo.

| Função | Tipo | O que faz |
|---|---|---|
| `projetil_adicionar` | pública | aloca uma baba com malloc na posição de disparo e a insere no INÍCIO da lista. |
| `remover_no` | interna | tira da lista o nó apontado por 'ligacao' (a cabeça ou o campo 'proximo' do nó anterior), religa a lista ao nó seguinte e libera o nó. |
| `projetil_atualizar_lista` | pública | move cada baba para a direita; se ela tocar um gato da sua linha, causa o dano nele (e, se for gelada, o deixa lento). |
| `projetil_desenhar_lista` | pública | desenha cada baba com a pixel-art do seu estilo (tabela de desenhos), centrada na posição dela. |
| `projetil_liberar_lista` | pública | remove a cabeça da lista até ela ficar vazia (NULL), liberando todos os nós. |

## `src/petisco.c`

Lista encadeada de petiscos.

| Função | Tipo | O que faz |
|---|---|---|
| `petisco_adicionar` | pública | aloca um petisco com malloc na posição dada (ele vai cair até a altura alvoY) e o insere no INÍCIO da lista: o novo nó aponta para a antiga cabeça e passa a ser a cabeça. |
| `remover_no` | interna | tira da lista o nó apontado por 'ligacao' (que é a cabeça da lista ou o campo 'proximo' do nó anterior), religando a lista ao nó seguinte, e libera a memória do nó removido. |
| `petisco_atualizar_lista` | pública | percorre a lista fazendo cada petisco cair até a sua altura alvo; depois de parado, conta o tempo no chão e remove os que passaram de DURACAO_NO_CHAO. Percorre com um ponteiro para ponteiro, o que permite remover um nó no meio da lista sem guardar o anterior. |
| `petisco_coletar` | pública | procura na lista um petisco sob o ponto clicado. |
| `petisco_desenhar_biscoito` | pública | desenha o biscoito em forma de osso centrado em 'centro', com cada pixel ampliado 'escala' vezes. |
| `petisco_desenhar_lista` | pública | desenha cada petisco como o biscoito ampliado, sobre um brilho amarelo que pulsa (para chamar a atenção do jogador), com sombra depois que pousa e, de vez em quando, a estrelinha brilhando na ponta, como no sprite do gerador. |
| `petisco_liberar_lista` | pública | remove a cabeça da lista até ela ficar vazia (NULL), liberando todos os nós. |

## `src/deck.c`

Tela de escolha do deck antes de cada fase.

| Função | Tipo | O que faz |
|---|---|---|
| `retangulo_opcao` | interna | área da carta da raça i (0 a 9) na grade de 2 x 5. |
| `retangulo_espaco` | interna | área do espaço i da linha "Seu deck" (0 = Pote de Ração). |
| `raca_da_opcao` | interna | tipo da raça da carta i da grade (pula o Pote, tipo 0). |
| `posicao_no_deck` | interna | posição da raça no deck escolhido, ou -1 se ela não está nele. |
| `remover_do_deck` | interna | tira a raça da posição dada e puxa as seguintes uma casa para trás, para o deck continuar sem buracos. |
| `alternar_raca` | interna | tira a raça do deck se ela já estiver nele; senão, a põe no fim (se ainda houver espaço). |
| `deck_iniciar` | pública | abre a tela para a fase pedida. |
| `deck_atualizar` | pública | ESC ou Voltar retornam ao mapa; Começar (ou ENTER) só funciona com o deck completo. |
| `desenhar_espacos` | interna | a linha "Seu deck": o Pote de Ração fixo no 1º espaço e os 5 espaços do deck, preenchidos na ordem em que as raças foram escolhidas. |
| `desenhar_opcao` | interna | carta de uma raça na grade: o cachorro inteiro em tamanho real (como no campo) em cima e, na faixa de baixo, o nome e o custo. |
| `desenhar_dica` | interna | abaixo da grade, o papel, o custo e a recarga da raça sob o mouse; sem nenhuma, quantas raças já foram escolhidas. |
| `deck_desenhar` | pública | cenário do menu, painel com o título, a linha "Seu deck", a grade com as 10 raças, a dica e os botões (Começar fica apagado até o deck estar completo). |

## `src/save.c`

Leitura e escrita dos saves em arquivo texto.

| Função | Tipo | O que faz |
|---|---|---|
| `save_limpar` | pública | zera todos os campos do save (espaço vazio, só a fase 1). |
| `save_novo` | pública | começa um jogo novo no save: limpo, mas marcado como usado. |
| `save_carregar_todos` | pública | lê os saves do arquivo, linha por linha, pulando as linhas de comentário (#). |
| `save_gravar_todos` | pública | reescreve o arquivo com um cabeçalho de comentário e uma linha por save. |
| `save_fases_concluidas` | pública | conta quantas posições de faseConcluida são true. |
| `save_registrar_pontos` | pública | se os pontos passarem do recorde da fase no save, troca o recorde e soma a diferença na pontuação do save (que é sempre a soma dos recordes, para repetir uma fase não inflar os pontos). |

## `src/highscore.c`

Leitura, escrita, inserção e desenho do Top 10.

| Função | Tipo | O que faz |
|---|---|---|
| `ranking_carregar` | pública | zera o ranking e lê o arquivo linha por linha, pulando as linhas de comentário (#). |
| `ranking_gravar` | pública | reescreve o arquivo com um cabeçalho de comentário e uma linha por recorde, já na ordem do ranking. |
| `ranking_posicao` | pública | procura o 1º lugar com menos pontos (num empate, quem chegou antes fica na frente). |
| `ranking_inserir` | pública | acha o lugar dos pontos, empurra os recordes de lá para baixo uma posição (o 10º sai, se a lista estiver cheia) e grava o novo recorde no lugar. |
| `ranking_desenhar` | pública | dentro da área, o cabeçalho das colunas e uma linha por recorde (o 1º lugar em laranja); sem recordes, uma mensagem. |

## `src/sprite.c`

Tabela dos spritesheets, carregamento e desenho dos quadros.

| Função | Tipo | O que faz |
|---|---|---|
| `sprite_carregar_todos` | pública | carrega como textura cada PNG da tabela que existir; os que faltarem ficam com id 0 (e o jogo usa as formas). |
| `sprite_descarregar_todos` | pública | libera as texturas carregadas. |
| `sprite_existe` | pública | retorna true se o PNG do sprite foi carregado. |
| `sprite_duracao_ate` | pública | soma o tempo dos quadros anteriores a 'quadro', ou seja, quantos segundos depois do início da animação esse quadro começa. |
| `sprite_duracao_total` | pública | segundos de um ciclo completo da animação. |
| `sprite_quadro_no_tempo` | pública | descobre qual quadro aparece 'tempo' segundos depois do início da animação, repetindo-a em loop: desconta a duração de cada quadro até sobrar menos do que a do quadro atual. |
| `sprite_desenhar` | pública | recorta o quadro do spritesheet (o quadro i começa em x = i * 64) e o desenha ampliado 'escala' vezes, com a origem do sprite sobre 'posicao' (arredondada, para os pixels não tremerem). |
| `sprite_desenhar_retrato` | pública | desenha só o retângulo 'retrato' do 1º quadro, em escala 1 (sem reduzir, para nenhum pixel sumir), centrado em 'centro'. |

## `src/som.c`

Tabela dos efeitos sonoros, carregamento e volume.

| Função | Tipo | O que faz |
|---|---|---|
| `som_carregar_todos` | pública | com o dispositivo de áudio aberto, carrega cada WAV da tabela que existir; os que faltarem ficam vazios. |
| `som_descarregar_todos` | pública | libera os sons carregados e os deixa vazios. |
| `som_tocar` | pública | toca o efeito no volume atual. |
| `som_definir_volume` | pública | muda o volume dos efeitos (limitado entre 0 e 1). |
| `som_volume` | pública | retorna o volume atual dos efeitos. |

## `src/audio.c`

Música de fundo: um loop chiptune em Lá menor, sombrio mas com um toque de "caixinha de música", composto aqui mesmo no código. melodia: onda quadrada com vibrato    arpejo: onda senoidal (oitava acima) baixo:   onda triangular              bateria: bumbo, caixa e chimbal (ruído) As amostras são somadas num vetor alocado dinamicamente, convertidas para um arquivo WAV em memória e entregues à raylib como stream de música.

| Função | Tipo | O que faz |
|---|---|---|
| `frequencia` | interna | converte um número de nota MIDI em hertz (Lá 69 = 440 Hz). |
| `ruido_branco` | interna | gerador xorshift de ruído entre -1 e 1. |
| `tocar_nota` | interna | soma uma nota ao buffer a partir de 'inicio' (segundos), com a forma de onda, o volume, o envelope e o vibrato pedidos. |
| `tocar_bumbo` | interna | soma ao buffer um bumbo (seno cujo tom cai rapidamente). |
| `tocar_caixa` | interna | soma ao buffer uma caixa (ruído com um pouco de tom grave). |
| `tocar_chimbal` | interna | soma ao buffer um chimbal curto. |
| `escrever_inteiro` | interna | grava 'valor' em 'bytes' bytes, do menos para o mais significativo (little-endian, como o formato WAV exige). |
| `gerar_musica_wav` | interna | compõe a música inteira (melodia, arpejo, baixo e bateria, compasso por compasso) num vetor de floats e monta um arquivo WAV completo em memória: cabeçalho de 44 bytes + amostras de 16 bits mono. |
| `audio_iniciar` | pública | abre o dispositivo de áudio e carrega a música: primeiro tenta um arquivo da equipe em assets/audio/; se não houver, gera a música em memória. |
| `audio_atualizar` | pública | deve ser chamada todo frame. |
| `audio_definir_volume` | pública | guarda o novo volume da música (limitado entre 0 e 1), tira do mudo e o aplica, se a música estiver tocando. |
| `audio_finalizar` | pública | para e descarrega a música, libera o WAV gerado e fecha o dispositivo de áudio. |

## `src/ui.c`

Elementos de interface reutilizados por todas as telas.

| Função | Tipo | O que faz |
|---|---|---|
| `ui_botao_clicado` | pública | verifica se o botão esquerdo do mouse foi pressionado sobre o botão neste frame. |
| `ui_desenhar_botao` | pública | desenha o botão com uma sombra "dura" de pixel-art. |
| `ui_desenhar_painel` | pública | desenha um retângulo escuro com borda, usado como fundo de caixas e janelas. |
| `ui_texto` | pública | escreve o texto com o canto superior esquerdo em (x, y) e uma sombra deslocada proporcional ao tamanho da fonte, para ficar legível em qualquer fundo. |
| `ui_texto_centro` | pública | igual a ui_texto, mas centraliza o texto em torno de centroX (use LARGURA_VIRTUAL / 2 para centralizar na tela). |
| `ui_desenhar_osso` | pública | desenha um osso (haste + 4 pontas redondas) centrado em 'centro'. |
| `ui_desenhar_barra_vida` | pública | barra fina centrada em 'centro', com a largura dada. |
| `ui_desenhar_pixel_art` | pública | desenha uma figura guardada como MATRIZ de caracteres (um vetor de strings, um caractere por pixel), com o canto superior esquerdo em 'canto' e cada pixel ampliado 'escala' vezes. |
| `ui_deslizador_atualizar` | pública | enquanto o botão esquerdo estiver apertado sobre a barra (com uma folga em volta, para ser fácil de pegar), o valor segue a posição do mouse: 0 na ponta esquerda e 1 na direita. |
| `ui_desenhar_deslizador` | pública | o rótulo à esquerda, a barra preenchida até o valor, o botão redondo na posição do valor e a porcentagem à direita. |

## `src/cenario.c`

Cenário do menu (pôr do sol sobre a cidade em ruínas) e o título do jogo. Os detalhes "aleatórios" usam cenario_ruido, que sempre devolve o mesmo número para a mesma entrada, então nada muda de lugar de um frame para o outro.

| Função | Tipo | O que faz |
|---|---|---|
| `cenario_ruido` | pública | embaralha três inteiros e devolve um número pseudoaleatório. |
| `cor_do_ceu` | interna | retorna a cor do degradê do céu na altura y, para um céu que vai de 0 até 'altura' (roxo no topo, rosa no meio, laranja no horizonte). |
| `cenario_desenhar_ceu` | pública | desenha o céu do pôr do sol de y = 0 até 'altura': degradê, estrelas piscando no topo, sol com brilho e faixas "retrô", nuvens que se movem devagar e corvos voando. |
| `cenario_desenhar_skyline_distante` | pública | desenha, apoiada na linha 'base', a silhueta roxa dos prédios ao longe, com antenas nos mais altos e poucas janelas acesas bem fracas. |
| `desenhar_janela` | interna | desenha uma janela em (jx, jy). |
| `desenhar_predio` | interna | desenha um prédio do primeiro plano apoiado em 'base': corpo (com topo desabado e vergalhões se for quebrado), sombra na lateral esquerda e luz do sol na direita, andares com janelas, porta de aço no térreo, rachadura em alguns, caixa d'água e antena piscando. |
| `desenhar_fogo` | interna | desenha um barril enferrujado em (x, base) com fogo: brilho no chão, chamas em colunas que oscilam com o tempo (laranja, amarelo e quase branco no centro) e faíscas subindo. |
| `desenhar_chao` | interna | desenha o chão do menu a partir da linha 'base': terra, meio-fio, rua rachada com faixa apagada, pedrinhas, grama seca, pegadas de pata, ossos, pneu velho, cerca quebrada, placa de perigo e o barril com fogo. |
| `cenario_desenhar_menu` | pública | desenha o cenário completo das telas de menu, de trás para a frente: céu, prédios distantes, prédios próximos e chão. |
| `titulo_desenhar` | pública | escreve "PawPocalypse" centralizado na altura y, em verde com sombra vermelho-escura. |

