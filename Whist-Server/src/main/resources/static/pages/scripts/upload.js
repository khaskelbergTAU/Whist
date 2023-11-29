function select_bot(bot)
{
    fetch('/run/select?bot=' + bot, {
        method: 'POST'
    }).then(response => {
        if (response.status != 200)
        {
            window.alert("Couldnt select bot:\n" + response.text());
            return;
        }
    }).then(_ => {
    let current_bot = document.getElementById("current_bot");
    if(current_bot != null)
    {
        let unselected_button = document.getElementById("non-selected-button").cloneNode(true);
        unselected_button.classList.remove("hidden");
        unselected_button.innerText = current_bot.innerText;
        unselected_button.id = current_bot.innerText;
        unselected_button.onclick = function() { select_bot(current_bot.innerText); };
        current_bot.replaceWith(unselected_button);
    }

    let button = document.getElementById(bot);
    let selected_button = document.getElementById("selected-button").cloneNode(true);
    selected_button.classList.remove("hidden");
    selected_button.innerText = bot;
    selected_button.id = "current_bot";
    button.replaceWith(selected_button);
    });
}

function load_bots()
{
    fetch('/run/bots', {
    method: 'GET',
    headers: {
        'Accept': 'application/json',
        'Content-Type': 'application/json'
    },
})
   .then(response => response.json())
   .then(response =>
    {
        let unselected_button = document.getElementById("non-selected-button");
        let selected_button = document.getElementById("selected-button");
        let botlist = document.getElementById("botlist");
        while(botlist.firstChild)
        {
            botlist.removeChild(botlist.firstChild);
        }
        if(response["selected"] != "")
        {
            let bot_button = selected_button.cloneNode(true);
            bot_button.classList.remove("hidden");
            bot_button.innerText = response["selected"];
            bot_button.id = "current_bot";
            botlist.appendChild(bot_button);
        }
        response["unselected"].forEach(bot => {
            if (bot == "")
            {
                return;
            }
            let bot_button = unselected_button.cloneNode(true);
            bot_button.classList.remove("hidden");
            bot_button.innerText = bot;
            bot_button.id = bot;
            bot_button.onclick = function() { select_bot(bot); };
            botlist.appendChild(bot_button);
        });
        return response;
    });
   //.then(response => console.log(JSON.stringify(response)));
}

function formSubmit() {
  let url = "/upload";
  let request = new XMLHttpRequest();
  let form = document.getElementById("bot-upload-form");
  request.open("POST", url, true);
  request.onload = function () {
    // request successful
    // we can use server response to our request now
    console.log(request.responseText);
  };

  request.onerror = function () {
    // request failed
  };

  request.send(new FormData(form));
}

function window_load()
{
    load_bots();
    setInterval(load_bots, 1000);
}

window.onload = window_load;