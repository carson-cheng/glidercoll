import lifelib
sess = lifelib.load_rules('b3s23')
lt = sess.lifetree()
from get_cost import getcost
#from shinjuku.gliderset import *
import time, requests
new = "results/new_synths.txt"
def upload_synthesis(rle):
    url = "https://catagolue.hatsya.com/postsynth"
    post = requests.post(url, data = {'content': rle})
    return post.text
def log_results(apgcode, rle, fn):
    fstream = open(fn, "a")
    if "x = 0" in rle:
        fstream.write(apgcode + "\n" + upload_synthesis(rle))
    else:
        fstream.write(apgcode + "\n" + upload_synthesis("x = 0, y = 0, rule = B3/S23\n" + rle))
    fstream.close()
# Source: https://www.w3resource.com/python-exercises/list/python-data-type-list-exercise-82.php
def combination(n, n_list):
    if n<=0:
        yield []
        return
    for i in range(len(n_list)):
        c_num = n_list[i:i+1]
        for a_num in combination(n-1, n_list[i+1:]):
            yield c_num + a_num
# end of quoted code
def gen_combs(num, upper_lim):
    combs = []
    for item in range(1, num-upper_lim+1):
        for comb in list(combination(item, list(range(num)))):
            combs.append(comb)
    return combs
report = ""
currentrle = ""
rles = []
rle_knowns = []
stopped = 1
knowns = []
num_searchers = int(input("Number of searchers: "))
num_gliders = int(input("Number of gliders: "))
combinations = gen_combs(num_gliders, 4)
#print(combinations)
def report_results(rlelist, cost, apgcode):
    #print(rlelist)
    #print("is the rle even being printed?")
    for item in rlelist:
        if "CLL" in item:
            # lifelib-type RLE with header
            print(item[:-1])
        else:
            # headerless RLE
            print("x = 0, y = 0, rule = B3/S23")
            print(item)
        if cost != None:
            for i in range(4, cost+1):
                if cost > i:
                    if lt.pattern(item).population == i * 5:
                        print("\u001b[32;1m\u001b[1mThis is a " + str(i) + "-glider synthesis.\u001b[0m")
                        log_results(apgcode, item, new)
def unique(arr):
    results = []
    for item in arr:
        if item not in results:
            results.append(item)
    return results
for item in range(num_searchers):
    fn = "results/" + str(num_gliders) + "g_test" + str(item+1) + ".txt"
    file = open(fn, "r").read()
    report = report + file
for line in report.split("\n"):
    if "x = 320, y = 320, rule = LifeHistory" in line:
        stopped = 0
    if "First" in line or "Previously" in line:
        if stopped == 0:
            rles.append(currentrle)
            #print(currentrle)
            currentrle = ""
        stopped = 1
    if stopped == 0:
        if "x = 320, y = 320, rule = LifeHistory" not in line:
            currentrle = currentrle + line
print("Processing RLEs, please wait...")
# detect removable glider (to find 4G and 5G syntheses)
for rle in rles:
    '''bbox = lt.pattern(rle).bounding_box
    if bbox is not None:
        if bbox[2] > 35 or bbox[3] > 35:
            print(rle)'''
    stabilized = lt.pattern(rle)[500]
    #print(rle)
    #try:
    apgcode = stabilized.apgcode
    '''gliders = gset.extract(lt.pattern(rle)).pairs()
    directions = []
    vectors = []
    for dir in range(len(gliders)):
        for item in gliders[dir]:
            directions.append(dir)
            vectors.append(item)
    for combo in combinations:
        reduced_gset = [[], [], [], []]
        for index in range(len(vectors)):
            if index not in combo:
                reduced_gset[directions[index]].append(vectors[index])
        string = ""
        for dir in range(len(reduced_gset)):
            for item in reduced_gset[dir]:
                string = string + str(item[0]) + " " + str(item[1]) + " "
            if dir < 3:
                string = string + "/"
        pat = gset.reconstruct(string).s
        pat = pat[500]
        if (pat.population == pat[1].population):
            if (pat == pat[1]):
                if apgcode == pat.apgcode:
                    rle = gset.reconstruct(string).s.rle_string()'''
    if apgcode not in knowns:
        knowns.append(apgcode)
        rle_knowns.append([rle])
    else:
        index = knowns.index(apgcode)
        rle_knowns[index].append(rle.split("!")[0] + "!")
print("Number of knowns " + str(len(knowns)))
for item in range(len(knowns)):
    apgcode = knowns[item]
    print(apgcode)
    try:
        if apgcode != "xs0_0":
            cost = getcost(apgcode)
            unknown = False
            if cost is not None:
                if cost > num_gliders:
                    unknown = True
            else:
                unknown = True
            if unknown == True:
                print("\u001b[32;1m\u001b[1mClean\u001b[0m glider synthesis found:")
                report_results(unique(rle_knowns[item]), cost, apgcode)
            else:
                if "Previously" not in rle_knowns[item]:
                    report_results(unique(rle_knowns[item]), cost, apgcode)
    except Exception as err:
        print(err)
        print("Error: There is a possible temporary problem with the network connection. Waiting for 30 seconds...")
        time.sleep(30)
