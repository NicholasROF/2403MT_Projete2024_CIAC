window.onload = function () {
    // Conectar ao broker EMQX
    const client = mqtt.connect('wss://broker.emqx.io:8084/mqtt'); // estabelece conexão com o broker no endereço

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
    });

    // Quando uma mensagem MQTT for recebida
    client.on('message', function (topic, message) {
        if (topic === "CIAC/ENER") {
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
    });
};
