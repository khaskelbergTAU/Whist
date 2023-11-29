function deselect_bot(bot)
{
    let bot_button = document.getElementById(bot);
    let unselected_button = document.getElementById("non-selected-button").cloneNode(true);
    unselected_button.classList.remove("hidden");
    unselected_button.innerText = bot;
    unselected_button.id = bot;
    unselected_button.onclick = function() { select_bot(bot); };
    bot_button.replaceWith(unselected_button);
}


function select_bot(bot)
{
    let button = document.getElementById(bot);
    let selected_button = document.getElementById("selected-button").cloneNode(true);
    selected_button.classList.remove("hidden");
    selected_button.innerText = bot;
    selected_button.id = bot;
    selected_button.classList.add("selected");
    button.replaceWith(selected_button);
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
        for(let child in botlist.children)
        {
            botlist.removeChild(child);
        }
        if(response["selected"] != "")
        {
            response["unselected"].append(response["selected"])
        }

        for(let bot in response["unselected"])
        {
            let bot_button = unselected_button.cloneNode(true);
            bot_button.classList.remove("hidden");
            bot_button.innerText = bot;
            bot_button.id = bot;
            bot_button.onclick = function() { select_bot(bot); };
            botlist.appendChild(bot_button);
        }
        document.getElementById("bottext").innerText = response;
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