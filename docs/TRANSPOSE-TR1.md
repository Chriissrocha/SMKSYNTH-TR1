# SMKSYNTH 1.3.1-TR1 — Transpose

Base: SMKSYNTH 1.3.1 Endless MIDI, commit `ddcb30101aea37d87b1dd942cba2294bf6a26a6e`.
TR1 é uma edição experimental separada, identidade 19669 para Pro e Elite. Os pacotes oficiais 1.3.1 (19668) e o instalador original não foram alterados.

## Como usar

Na tela Performance (PATCH), mantenha SCALE pressionado e aperte OCTAVE + ou OCTAVE −. Cada toque altera um semitom; o visor informa `TRANSPOSE +1`, `TRANSPOSE -1` etc. Limite de −12 a +12. SCALE com ambos os OCTAVE retorna a zero. OCTAVE sem SCALE conserva a função original. A transposição soma-se às oitavas do timbre e do Split/Layer e permanece durante a sessão; ao reiniciar o instrumento, inicia em zero. Não há alterações no formato dos presets, patterns, favoritos ou globais.

Somente notas novas das teclas físicas para a engine ANALOG recebem o deslocamento. Notas já pressionadas conservam a altura e sua liberação original. As notas recebidas por MIDI, DRUM-X, os patterns existentes, a transposição do sequenciador/ENDLESS e o MODO MIDI dedicado não são alterados. Gravações novas armazenam a nota ouvida, como na função de oitavas original. A saída MIDI no modo synth continua espelhando o som produzido (portanto acompanha a nota transposta). O atalho fica restrito à tela Performance para preservar as outras ações de SCALE e OCTAVE.

## Build e instalador

A prévia do instalador web está em `web/install-transpose/` e fica bloqueada enquanto não houver os quatro pacotes TR1 reais. Não use os firmwares 1.3.1 já existentes no projeto esperando obter Transpose.

Em Linux x86-64 (Windows: WSL2 Ubuntu), instale Python 3, Pillow, compilador C, Node 22, Git e curl. Obtenha o toolchain e o SDK oficiais conforme os scripts originais:

```sh
sh tools/get_toolchain.sh
git clone --depth 1 --branch AC79NN_SDK_V1.2.1_2023-12-13 https://gitee.com/Jieli-Tech/fw-AC79_AIoT_SDK.git "$HOME/fw-AC79_AIoT_SDK"
python3 -m pip install Pillow
export JIELI_TOOLCHAIN="$HOME/.jieli/toolchain"
export AC79_SDK="$HOME/fw-AC79_AIoT_SDK"
python3 tools/build_transpose_release.py
```

Alternativa: executar manualmente o workflow `Build SMKSYNTH TR1 (private candidate)` em um repositório separado, com os workflows antigos de publicação automática desabilitados. Ele compila e testa e disponibiliza o artefato; não publica release nem faz instalação em teclado.

Após build bem-sucedido, `build/transpose-dist/` contém os quatro `.fwsc` novos, manifesto SHA-256, instalador e fontes correspondentes. Sirva localmente o diretório:

```sh
python3 -m http.server 8000 --bind 127.0.0.1 --directory build/transpose-dist
```

Abra `http://localhost:8000/install/` no Chrome/Edge do computador. A página identifica Pro/Elite e escolhe primeira instalação a partir da V016 ou atualização com preservação dos dados do SMKSYNTH. Faça backup antes e siga os avisos do instalador. Se uma transferência for interrompida, use o mesmo instalador, navegador e origem. Não use a versão oficial anterior como retomada de uma instalação TR1.

**Estado da entrega:** código e testes de host executados; compilação do firmware e testes no teclado físico não realizados no ambiente de entrega, pois o toolchain/SDK específicos não estavam disponíveis. A edição não tem aprovação em hardware. Não instale sem verificar os resultados do build e fazer backup. O simulador WASM oficial 1.3.1 permanece o original; não foi recompilado para TR1.
