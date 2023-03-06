#apgcode = input("Please input the apgcode. ")
def getcost(apgcode):
    if apgcode[1] == "s":
        sl_size = int(apgcode.split("s")[1].split("_")[0])
    else:
        sl_size = 0
    import requests
    synthesis = requests.get("https://catagolue.hatsya.com/textsamples/" + apgcode + "/b3s23/synthesis").text
    try:
        cost = synthesis.split("costs ")[1].split(" gliders")[0]
        if int(cost) > sl_size and sl_size > 0:
            print("This object costs " + "\u001b[33;1m\u001b[1m" + cost + "\u001b[0m gliders.")
        else:
            print("This object costs " + "\u001b[32;1m\u001b[1m" + cost + "\u001b[0m gliders.")
        return int(cost)
    except:
        print("This object is \u001b[31m\u001b[1munsynthesized\u001b[0m.")
        return None
