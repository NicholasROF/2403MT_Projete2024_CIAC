         // Conectar ao broker EMQX
         const client = mqtt.connect('wss://broker.emqx.io:8084/mqtt'); //estabelece conexão com o broker no endereço

         client.on('connect', function () { //conexão com o broker for bem sucedida
             console.log('Conectado ao broker EMQX'); //exibe console no navegador
             client.subscribe("CIAC/TEMP", function (err) { //fornece mensagem relacionada a temperatura
                 if (!err) { //se não houver erro 
                     console.log('Inscrito no tópico Temperatura/Nick'); //exibir mensgem
                 }
             });
             client.subscribe("CIAC/PRES", function (err) { //fornece mensagem relacionada a presença
                 if (!err) { //se não houver erro
                     console.log('Inscrito no tópico Temperatura/Nick'); //exibir mensagem
                 }
             });
             client.subscribe("CIAC/ENER", function (err) { //fornece mensasgem relacionada ao gasto energético
                 if (!err) { //se não houver erro
                     console.log('Inscrito no tópico Temperatura/Nick'); //exibe mensagem
                 }
             });
             client.subscribe("CIAC/LUMI", function (err) { //fornece mensagem relacionada a temperatura
                 if (!err) { //se não houver erro 
                     console.log('Inscrito no tópico Temperatura/Nick'); //exibir mensgem
                 }
             });

             client.subscribe("CIAC/ARCON", function (err) { //fornece mensagem relacionada a temperatura
                 if (!err) { //se não houver erro 
                     console.log('Inscrito no tópico Temperatura/Nick'); //exibir mensgem
                 }
             });
         });
        
        var horario = [];

         client.on('message', function (topic, message) { //cliente recebe mensagem
            if(topic == "CIAC/TEMP"){ //mensagem de temperatura
                // Atualiza a temperatura na interface
                document.getElementById('mensagemTemp1').textContent = `${message.toString()} °C`; 
                document.getElementById('mensagemTemp2').textContent = `${message.toString()} °C`; 

                // Converte a mensagem recebida para número para fazer a comparação
                const temperatura = parseFloat(message.toString());

                // Verifica se a temperatura é maior ou igual a 24 para ligar o ar-condicionado
                if (temperatura >= 24) {
                    document.getElementById('mensagemArcon').textContent = "Ligado";
                } else {
                    document.getElementById('mensagemArcon').textContent = "Desligado";
                }
            }
            if(topic == "CIAC/PRES"){ // mensagem de presença
                document.getElementById('mensagemPres').textContent = `${message.toString()}`; 
            }
            if(topic == "CIAC/ENER"){ // mensagem de gasto enegético

                const agora = new Date();
                const dias = agora.getDay();
                const horas = agora.getHours();
                const minutos = agora.getMinutes();
                

                horario.push(horario);

                console.log(`Horário atual: ${dias}:${horas}:${minutos}:`);

                document.getElementById('mensagemEner1').textContent = `${message.toString()} w/h`; 
                document.getElementById('mensagemEner2').textContent = `${message.toString()} w/h`; 
            }
            if(topic == "CIAC/LUMI"){
                document.getElementById('mensagemLumi').textContent = `${message.toString()} `;
            }
            });