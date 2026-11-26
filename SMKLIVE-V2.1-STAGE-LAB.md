# SMKLIVE V2.1 — Stage Lab

V2.1 e a versao do testador web. O firmware e o motor C permanecem V2 (1.3.1-live2, Pro 19701 / Elite 19702). Nenhum binario de hardware foi compilado neste ambiente. O arquivo HTML inclui o mesmo WASM V2, sem dependencia externa para simular.

## Publicar online

1. Envie o conteudo deste repositorio ao GitHub, inclusive `.github/workflows/`.
2. Em Settings > Pages, escolha GitHub Actions como fonte.
3. Em Actions > SMKLIVE - publish Stage Lab (manual), execute Run workflow. A URL publicada aparece no resultado de Deploy. A pagina raiz sera o testador; nao sao publicados os instaladores ou firmwares historicos. A ferramenta de backup original, para SMKSYNTH 19668, fica em `/backup/`.
4. Abra a URL HTTPS no Chrome ou Edge atualizado. O site ainda nao foi publicado por este trabalho.

Tambem pode abrir `web/smklive-tester/index.html` diretamente para testar o motor e os controles PC. Para permissoes MIDI/SysEx, use HTTPS ou localhost (por exemplo, com Python instalado, na pasta `web/smklive-tester`, `python -m http.server 8000`, e acesse `http://localhost:8000/`). O suporte direto file:// depende do navegador.

## MIDI real

- Clique Ativar audio para ouvir a sintese no PC. Conecte o SMK por USB de dados, clique Conectar MIDI e permita acesso. Escolha a entrada se houver mais de uma. Nenhum MIDI e enviado ao instrumento pelo modo de simulacao.
- A primeira tecla virtual e MIDI 48 por padrao. Se o teclado fisico estiver em outra oitava, clique Calibrar tecla grave e toque a primeira tecla. A faixa de 37 notas e mapeada para os eventos fisicos virtuais: transpose, zonas e layers do firmware funcionam nesse caminho. O filtro de canal e opcional.
- Sustain (CC64), modulacao (CC1) e pitch bend sao recebidos automaticamente. A velocidade de Note On e repassada. Note Off e Note On com velocidade zero liberam a nota. MIDI + mouse + teclado PC podem tocar juntos; a mesma tecla permanece ativa ate todas as fontes soltarem. Desconectar libera as notas e modificadores MIDI, sem apagar timbres.
- Para mapear faders/knobs/pads/botoes, abra MIDI Learn, selecione o alvo virtual, clique Aprender controle, mova/pressione o controle real. Knobs absolutos usam a diferenca entre valores consecutivos; relativos suportam 1..63 / 65..127 em complemento de dois. Faders aceitam CC absoluto. Botoes/pads aceitam Note On/Off ou CC de limiar 64. Mapas ficam na sessao da entrada; remapeie ao reconectar. Controles que nao emitem MIDI nao podem ser reconhecidos. Aftertouch nao foi mapeado nesta versao. O teclado fisico nao esta executando SMKLIVE por estar conectado ao testador.

## PC

- Segure A para GLOBE, Shift para SEQ, F para FX, L para PARA, P para PATCH. Gire qualquer knob com a rodinha do mouse, arraste ou clique em um de seus lados. Alternativamente, use a faixa Combinações no PC: Segurar GLOBE/SEQ/etc., gire o knob, depois Soltar. Clique direito em qualquer botao virtual tambem trava/destrava. Setas esquerda/direita sao os botoes fisicos; [ e ] giram o encoder selecionado. K/O = octave -/+. Espaco = sustain; Esc = panic.
- Z S X D C V G B H N J M e Q 2 W 3 E R 5 T 6 Y 7 U I tocam as duas oitavas do teclado PC; selecione a faixa C3-C5 ou C4-C6. Perder foco dispara panic por seguranca. Mouse e teclado compartilham o mesmo motor WASM.
- Para mudar o tema: toque GLOBE ate THEME, use K1. Transpose continua -12 a +12 por semitom, segurando SEQ + K1/setas. O banco de faders C controla volumes A-D. Zonas independentes continuam de sessao.

## Instalacao USB

O painel integrado usa o protocolo SysEx original do projeto. A simulacao e a gravacao sao separadas. A gravacao nao e habilitada sem pacote, manifesto e identidade compativeis, backup declarado e confirmacao. O manifesto e gerado por `build-live.sh` ao final do build e permite verificar tamanho, SHA-256, identidade e CRC interno; nao e assinatura de autenticidade. Use exclusivamente arquivos do seu build confiavel. Nunca modifique identidades de pacotes.

1. Compile no GitHub Actions > SMKLIVE - experimental update build (forneca SDK de referencia e modelo), como ja documentado no guia original. O artefato gera `.fwsc`, `.json` e `.sha256` correspondentes. Nao ha binario V2 instalavel neste ZIP.
2. A origem aceita e deliberadamente restrita ao SMKSYNTH 1.3.1 (19668) ou ao SMKLIVE V2 do mesmo modelo. Para firmware M-VAVE V016, use o fluxo oficial e o instalador SMKSYNTH original em separado antes de atualizar; o update integrado nao e pacote de primeira instalacao.
3. Salve e exporte o backup real do banco antes de atualizar. O backup original publicado aceita 19668; o suporte a exportacao de um teclado que ja esta em V2 nao foi certificado aqui. Nao marque a caixa se nao tiver backup. Guarde tambem o arquivo oficial V016 correspondente e M-UPGRADE para recuperacao.
4. No testador, escolha o `.fwsc` e o `.json` do mesmo artefato. Desconecte outros dispositivos MIDI e feche DAWs. Conecte por USB de dados, alimentacao estavel. Clique Identificar teclado USB. A conexao MIDI de simulacao e encerrada e o audio suspenso antes da consulta/gravacao.
5. Confira modelo, origem, destino e pacote. Marque as confirmacoes e clique Atualizar / retomar. A operacao usa exclusao mutua entre abas da mesma origem, valida a identidade novamente antes da gravacao e exige o loader exato Pro ou Elite. Nao desconecte nem feche a pagina; aguarde identidade final. Os logs podem ser baixados.
6. Se interromper, nao use pacote de outro modelo nem de outra versao. Abra o mesmo endereco/navegador, selecione os mesmos arquivos, consulte e retome. Se a operacao anterior foi iniciada por outro instalador, retome nele. USB RESCUE: somente sem gravacao em andamento, tente ligar segurando exclusivamente OCTAVE -. O caminho original foi mantido, mas nao e uma garantia contra todas as falhas de boot. Consulte docs/INSTALLATION.md.

## Validacao e limites

- Motor WASM V2 original preservado. Testes de semitons, quatro zonas, faders, efeitos e panic executados no Chromium.
- Gestos de mouse, Shift + K1, sticky GLOBE + scroll, entrada MIDI mock, Note On/Off, Note On vel 0, duas fontes de uma mesma nota, MIDI Learn de fader, liberacao ao desconectar e bloqueio inicial do instalador: testados em Chromium com HTML carregado diretamente em memoria. O navegador do ambiente bloqueou navegacao real file:// e localhost, portanto permissoes reais e acesso fisico ainda devem ser testados.
- `node tests/smklive_web_install_test.mjs`: testou pacote historico recusado, SHA/CRC/identidade, modelo/origem incompatíveis, loader incorreto, falha e retomada com o mesmo hash. Teste mock, nao escreveu hardware. Fixture sintetica em memoria nunca distribuida.
- GitHub Pages workflow incluido, nao executado; compilacao JieLi, USB real, audio fisico, recuperacao e instalacao fisica pendentes. Nao ha garantia de recuperacao em qualquer falha.
