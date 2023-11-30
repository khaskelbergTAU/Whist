from selenium import webdriver
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support.ui import Select
from selenium.webdriver.common.by import By
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.common.action_chains import ActionChains
import time
import sys

if len(sys.argv) != 9:
    print("Usage: python3 troll.py <group1name> <group2name> <group3name> <group4name> <message spacing> <url> <session_id> run_count")
    exit()
spacing = int(sys.argv[5])
url = sys.argv[6]
session_id = sys.argv[7]
run_count = int(sys.argv[8])
driver = webdriver.Remote(command_executor=url,
                          options=webdriver.ChromeOptions())
driver.close()
driver.session_id = session_id
driver.get("https://web.whatsapp.com")
print("go to nuki, And then Ctrl^C")

print("starting:")
time.sleep(20)
i = 0
s = ""
while True:
    s = input()
    if i == run_count - 1:
        break
    if i % spacing != 0:
        i += 1
        continue
    i += 1

    scores = s.strip().split(",")
    if len(scores) != 4:
        print("wrong input")
        continue
    names_and_scores = [f"{sys.argv[i]} {scores[i - 1]}" for i in range(1, 5)]
    msg = f"round {i}:"
    actions = ActionChains(driver)
    actions = actions.send_keys(msg).key_down(
        Keys.SHIFT).send_keys(Keys.ENTER).key_up(Keys.SHIFT)
    for inp in names_and_scores:
        actions = actions.send_keys(inp)
        actions = actions.key_down(Keys.SHIFT).send_keys(
            Keys.ENTER).key_up(Keys.SHIFT)
    actions.send_keys(Keys.ENTER)
    actions.perform()


scores = s.strip().split(",")
names_and_scores = [f"{sys.argv[i]} {scores[i - 1]}" for i in range(1, 5)]
msg = "final scores:"
actions = ActionChains(driver)
actions = actions.send_keys(msg).key_down(
    Keys.SHIFT).send_keys(Keys.ENTER).key_up(Keys.SHIFT)
for inp in names_and_scores:
    actions = actions.send_keys(inp)
    actions = actions.key_down(Keys.SHIFT).send_keys(
        Keys.ENTER).key_up(Keys.SHIFT)
actions.send_keys(Keys.ENTER)
actions.perform()
