function deselect_bot(bot)
{
    let bot_button = document.getElementById(bot + "_selected");
    bot_button.remove();
}


function select_bot(bot)
{
    let button = document.getElementById("non-selected-button").cloneNode(true);
    button.classList.remove("hidden");
    button.innerText = bot;
    button.id = bot + "_selected";
    button.classList.add("selected");
    button.onclick = function() { deselect_bot(bot); };
    document.getElementById("current_bots").appendChild(button);
}

function start_run()
{
    let bots = [];
    let botlist = document.getElementById("current_bots");
    botlist.childNodes.forEach(bot => {
        bots.push(bot.innerText);
    });
    if(bots.length != 4)
    {
        alert("Please select 4 bots to run.");
        return;
    }
    run_count_input = document.getElementById("run_count");
    if(run_count_input.value == "")
    {
        alert("Please enter a run count.");
        return;
    }
    let run_count = parseInt(run_count_input.value);
    if(isNaN(run_count))
    {
        alert("Please enter a valid run count.");
        return;
    }

    fetch('/run/start', {
    method: 'POST',
    headers: {
        'Accept': 'application/json',
        'Content-Type': 'application/json'
    },
    body: 
    {
        "bots": bots.join(","),
        "run_count": run_count,
    }
    }).then(response => {
        if(response.ok)
        {
            window.location.href = "/scores.html";
        }
        else
        {
            response.text().then(text => {
            alert("Error starting run:\n" + text)
        });
        }
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
        let botlist = document.getElementById("botlist");
        while(botlist.firstChild)
        {
            botlist.removeChild(botlist.firstChild);
        }
        let bots = []
        response["unselected"].forEach(bot =>
            {
                if(bot != "")
                {
                    bots.push(bot);
                }
            });
        if(response["selected"] != "")
        {
            bots.push(response["selected"]);
        }
        bots.forEach(bot => {
            if(bot == "")
            {
                return;
            }
            let unselected_button = document.getElementById("non-selected-button");
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


function window_load()
{
    load_bots();
    setInterval(load_bots, 1000);
}

window.onload = window_load;