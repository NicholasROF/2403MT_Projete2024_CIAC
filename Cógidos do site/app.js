var shut = false;
var estadoLuminosidade = false;
var estadoPresenca = true;
var tempAMB;
var tempON = 20;
var tempOFF = 16;

var ligado = true;

// Conectar ao broker EMQX
const client = mqtt.connect('wss://broker.emqx.io:8084/mqtt'); // estabelece conexão com o broker no endereço

window.onload = function () {

    // Variáveis para armazenar os dados do gráfico
    let energiaValores = [];  // Armazenará os valores de energia recebidos
    let horarioLabels = [];   // Armazenará os horários em que os valores foram recebidos

    // Configuração inicial do gráfico (vazio)
    const ctx = document.getElementById('energyChart').getContext('2d'); 
    const energyChart = new Chart(ctx, {
        type: 'line',  // Mantenha o tipo de gráfico como linha
        data: {
            labels: horarioLabels,  // Iniciar com labels vazios
            datasets: [{
                label: 'Gastos de Energia (Wh)',
                data: energiaValores,  // Iniciar com dados vazios
                backgroundColor: 'rgba(54, 162, 235, 0.2)',  // Cor de fundo para a linha
                borderColor: 'rgba(54, 162, 235, 1)',        // Cor da linha
                borderWidth: 2,  // Espessura da linha
                fill: true,      // Preencher a área abaixo da linha
                tension: 0.4     // Suavizar a linha
            }]
        },
        options: {
            scales: {
                y: {
                    beginAtZero: true, // Começa o eixo Y a partir de 0
                    ticks: {
                        color: '#ffffff' // Cor dos elementos do eixo Y (branco)
                    }
                },
                x: {
                    ticks: {
                        color: '#ffffff' // Cor dos elementos do eixo X (branco)
                    }
                }
            },
            plugins: {
                legend: {
                    labels: {
                        color: '#ffffff' // Cor da legenda (branco)
                    }
                }
            }
        }
    });

    // Atualiza o gráfico com novos valores
    function updateEnergyChart(valorEnergia, horario) {
        // Adiciona o novo valor e o horário correspondente
        energiaValores.push(valorEnergia);
        horarioLabels.push(horario);

        // Limita a quantidade de pontos a serem exibidos (por exemplo, últimos 10)
        if (energiaValores.length > 10) {
            energiaValores.shift();  // Remove o primeiro valor
            horarioLabels.shift();   // Remove o primeiro horário
        }

        // Atualiza o gráfico
        energyChart.update();
    }
    // Quando o cliente MQTT conectar
    client.on('connect', function () {
        console.log('Conectado ao broker EMQX');
        
        // Inscrever-se no tópico CIAC/ENER para receber o consumo energético
        client.subscribe('CIAC/ENER', function (err) {
            if (!err) {
                console.log('Inscrito no tópico CIAC/ENER');
            }
        });
        client.subscribe("CIAC/TEMP", function (err) {
            if (!err) {
                console.log('Inscrito no tópico CIAC/TEMP');
            }
        });
        client.subscribe("CIAC/PRES", function (err) {
            if (!err) {
                console.log('Inscrito no tópico CIAC/PRES');
            }
        });
        client.subscribe("CIAC/LUMI", function (err) {
            if (!err) {
                console.log('Inscrito no tópico CIAC/LUMI');
            }
        });
    });

    // Quando uma mensagem MQTT for recebida
    client.on('message', function (topic, message) {
        if (topic === "CIAC/ENER") {
            document.getElementById('mensagemEner1').textContent = `${message.toString()} Wh`;
            document.getElementById('mensagemEner2').textContent = `${message.toString()} Wh`;

            const consumoEnergia = parseFloat(message.toString());  // Converte a mensagem para um número
            
            // Obtém o horário atual
            const agora = new Date();
            const horario = `${agora.getHours()}:${agora.getMinutes()}:${agora.getSeconds()}`; // Exemplo de formato de horário
            
            // Atualiza os elementos de consumo energético na interface
            document.getElementById('mensagemEner1').textContent = `${consumoEnergia} Wh`; 
            document.getElementById('mensagemEner2').textContent = `${consumoEnergia} Wh`;

            // Atualiza o gráfico com o novo valor e horário
            updateEnergyChart(consumoEnergia, horario);
        }
        if (topic == "CIAC/TEMP") {
            // Converte a mensagem recebida para número
            const temperatura = parseFloat(message.toString());
            tempAMB = temperatura;
            // Atualiza a temperatura na interface
            document.getElementById('mensagemTemp1').textContent = `${temperatura} °C`;
            document.getElementById('mensagemTemp2').textContent = `${temperatura} °C`;
        }
        if (topic == "CIAC/PRES") {
            // Atualiza a presença na interface
            const presenca = parseInt(message.toString());
            if(presenca == 1){
                estadoPresenca = true
                document.getElementById('mensagemPres').textContent = "Sim";   
            } else {
                estadoPresenca = false
                document.getElementById('mensagemPres').textContent = "Não";   
            }
        }
        if (topic == "CIAC/LUMI") {
            const luminosidade = parseInt(message.toString());
            if(luminosidade == 1){
                estadoLuminosidade = true;
            } else{
                estadoLuminosidade = false;
            }
        }
        atualizaStatus();
    });
};

function ligar() {
    client.publish('CIAC/ON_LUMI', "1"); // Publica para desligar a luz
    }

    function desligar() {
    client.publish('CIAC/ON_LUMI', "0"); // Publica para desligar a luz
    }

    // Função de Shutdown
    var desligado = false;
    function shutdownSystem() {
    if (!desligado){
    // Desligar a luz
    alert("Sistema em Shutdown. Luz e Ar-condicionado serão desligados.");

    document.getElementById("mensagemLumi").textContent = "Desligado"; // Atualiza status da luz para Desligado
    document.getElementById("mensagemAIR_STATUS").textContent = "Desligado"; // Atualiza status do ar-condicionado para Desligado

    // Enviar mensagens de desligamento via MQTT
    client.publish('CIAC/SHUT', "1"); // Publica para desligar a luz e o ar-condicionado

    desligado = true;
    shut = true;
    } else{
        
        // Desligar a luz
        alert("Sistema em Shutdown. Luz e Ar-condicionado ligados.");
        document.getElementById("mensagemLumi").textContent = "Ligado"; // Atualiza status da luz para Ligado
        document.getElementById("mensagemAIR_STATUS").textContent = "Ligado"; // Atualiza status do ar-condicionado para Ligado
    

        // Enviar mensagens de desligamento via MQTT
        client.publish('CIAC/SHUT', "0"); // Publica para desligar a luz

        desligado = false;
        shut = false;
    }
    }

    function sendMessageTempCont() {
    const messageInput = document.getElementById('messageInput');
    const message = messageInput.value;
    client.publish('CIAC/AIR_TEMP', message);

    // Atualiza a célula de análise de dados para o parâmetro Ar Condicionado
    document.getElementById('mensagemAIR_TEMP').textContent = message + " °C";

    messageInput.value = ''; // Limpa o campo de entrada
    atualizaStatus();
    }

    function sendMessageTempON() {
    const messageInput = document.getElementById('tempONInput');
    const message = messageInput.value;
    tempON = message;
    client.publish('CIAC/AIR_CONT_ON', message);
    atualizaStatus();
    }

    function sendMessageTempOFF() {
    const messageInput = document.getElementById('tempOFFInput');
    const message = messageInput.value;
    tempOFF = message;
    client.publish('CIAC/AIR_CONT_OFF', message);
    atualizaStatus();
    }

    function atualizaStatus(){
        console.log(estadoPresenca);
        console.log(estadoLuminosidade);
        console.log(tempAMB);
        console.log(shut);
        console.log(tempOFF);
        console.log(tempON);

        if(estadoPresenca && !shut && !estadoLuminosidade){
            document.getElementById('mensagemLumi').textContent = "Ligado";
        } else {
            document.getElementById('mensagemLumi').textContent = "Desligado";
        }

        if(estadoPresenca && !shut){
            // Histerese: Muda o estado apenas quando os limites forem ultrapassados
            if (tempAMB >= tempON && !ligado) {
                ligado = true;
                document.getElementById('mensagemAIR_STATUS').textContent = "Ligado"; // Ar-condicionado ligado
                console.log("Ar-condicionado Ligado");
            } else if (tempAMB <= tempOFF && ligado) {
                ligado = false;
                document.getElementById('mensagemAIR_STATUS').textContent = "Desligado"; // Ar-condicionado desligado
                console.log("Ar-condicionado Desligado");
            }
        } else{
        ligado = false;
        document.getElementById('mensagemAIR_STATUS').textContent = "Desligado"; // Ar-condicionado desligado
        console.log("Ar-condicionado Desligado");
        }
    }
