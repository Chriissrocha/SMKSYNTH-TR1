# Validacao SMKLIVE V2

- Teste nativo C dedicado: aprovado (transpose de 25 posicoes, reset, K1, quatro zonas, note-off, faders, pads, FX, panic, ABI, predicado de recuperacao, temas).
- Checagem de fronteira HAL/MMIO: aprovada.
- Simulador: firmware C compilado em WebAssembly, HTML independente. Testes internos e interacao de painel no Chromium aprovados; a abertura direta por file:// nao foi testada devido a politica do ambiente. USB, ADC, Bluetooth e boot fisico nao sao simulados integralmente.
- A remocao do pool de bateria representa 2688 bytes no layout WASM; nao equivale a economia liquida no alvo. Endless nao e executado no perfil LIVE. Nao ha mapa de memoria JieLi comparativo.
- Quatro zonas: configuracao de sessao, nao gravada nas cenas. O limite de polifonia do motor permanece compartilhado entre partes.
- Toolchain JieLi e teclado fisico indisponiveis. Nenhum binario V2 instalavel foi validado ou fornecido. Nao instalar pacotes historicos como se fossem V2. Preservar backup e testar recuperacao antes de atualizar.
