Aeroporto
=========

Introdução
---------
Tradicionalmente, programas de computador são o sequenciais e possuem apenas um fluxo de execução. Problemas que exigiam paralelismo para se alcançar sua solução começaram a ser atacados a medida que a Ciência da Computação evoluiu, mediante mecanismos para sincronizacçaão entre processos sobre uma memória compartilhada. Visando exercitar a Programação Concorrente, este trabalho então expõe um problema envolvendo os conceitos de Comunicação entre Processos sobre Memoria Compartilhada e propõe uma solução para o mesmo.


Formalização
-----------
Em aeroportos o controle de tráfico aéreo é um serviço prestado por controladores, em terra, que acompanham, orientatam e monitoram o percurso  de aeronaves no ar e no solo, para garantir  um fluxo de tráfico seguro e ordenado. Torres de controle fornecem indicaçoões de voo, de acordo com as características operacionais das aeronaves e as condições de tráfego em determinado momento. Esse trabalho tem como objetivo  a implementação de um sistema que simule os eventos de uma aeronava que voa entra dois aeroportos e recebe todas as informação de tomada de decisão de uma torre de controle. A torre de controle é encarregada de fornecer informações referentes a utilização da pista de pouso, portão de embarque, portão de desembarque, atribuição de equipe de solo, dentre outras operações.

Descrição do Algoritmo
---------------------

##Estruturas

###Aviao:
Cada aviao possui um identificador para marca-los, assim como possuem um estado para controle a partir de uma máquina de estados. Este estado pode ser um entre os seguintes, VOANDO, QUEBRADO, GARAGEM, DECOLANDO, POUSANDO, ABASTECENDO, DESEMBARQUE, EMBARQUE e POUSO_FORCADO. Dentre estes o estado QUEBRADO não possui implementação por questões de tempo e por não ter sido especificado sua necessidade.
Além disso uma instância avião possui também um marcador de combustível, um aeroporto_atual para saber-se em qual aeroporto o avião em questão se encontra, assim como dois outros marcadores para ver se o avião já foi preparado para o próximo vôo, limpo, e um para checar se ainda é o primeiro vôo do avião, first.
Este tipo de dado conta também com uma função init para inicializar os dados básicos do avião com seus valores padrões e uma destroy que faz o contrário liberando memória se necessário.

###Aeroporto:
Cada aeroporto por sua vez possui um mutex para pista, pois foi-se especificado que só existiria uma pista, um semáforo para portões, inicializado com 20, assim como uma para equipes terrestres, inicializado com 10. Por fim foi-se necessário um marcador para pousos emergênciais e um para pousos normais assim como suas variáveis de condição para garantir a espera de outras aeronaves.
Este tipo de dado assim como o anterior conta com duas funções, init e destroy.

##Implementação

###main:
Serve apenas para chamar a inicialização dos objetos e criar as threads iniciais dos aviões.

torre_controle:
Todo avião utiliza a torre_controle para ser controlado, a partir de uma máquina de estados a torre controla o avião e sincroniza os outros em caso de pouso de emergencia ou decolagem enquanto há necessidade de pousos convencionais.
Os aviões começam no estado garagem e após serem limpos, caso haja a necessidade, são colocados em modo de embarque. Os aviões esperam a limpeza através de uma variável de condição seguindo o padrão utilizado para espera de pouso emergencial que será explicado abaixo.
Para a sincronização é utilizado uma variável de condição que espera enquanto há pousos de emergência, tanto no caso de decolagem quanto no caso de pouso convencional, é utilizado um laço para checar se ocorreu algum novo pouso de emergência, caso não pode seguir o funcionamento de pouso ou decolagem. Porém no caso de decolagem há a prioridade de pousos emergênciais porém também existe a de pousos convencionais, afim de se gastar menos combustível de aeronaves em vôo. Portanto após a espera do pouso emergencial é necessário fazer o mesmo processo para pousos normais.
Agora que os aviões estão em espera é preciso liberá-los no momento correto, no caso de aviões em pouso emergencial ao aterrisarem o marcador de emergência é voltado para 0 e então ocorre um broadcast na variável condicional de pouso de emergência. Utiliza-se o mesmo princípio no pouso normal, porém o broadcast ocorre na variável de pouso convencional e o marcador de pouso é setado para 0.
Após o embarque os aviões são colocados em modo de vôo e possuem uma chance de 20% para precisarem de um pouso forçado, num pouso forçado o avião gasta metade do combustível e não muda de aeroporto. Já no pouso normal é utilizado 1/4 do combustível máximo e muda de aeroporto.
Ao terminar de voar é colocado em modo de pouso e após isto em modo desembarque. Neste último modo é criada uma nova thread para preparação para o próximo vôo afim de tornar este processo concorrente ao embarque/desembarque, porém ela é sincronizada a partir de um signal para a variável de condição no modo garagem afim de somente liberar o embarque após o avião estar preparado para vôo.

###prepara_voo:
Esta função é utilizada pela modo desembarque para preparar o avião, é chamada uma equipe terrestre que fica encarregada de limpar e abastecer o avião caso necessário, possui sincronização com o modo garagem, liberando para embarque somente ao término da chamada.
