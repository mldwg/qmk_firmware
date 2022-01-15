import re

s = "k13, k16, k11, k21, k00, k06, k26, k17, k10, k12, k14, k90, k36, k27, k20, k22, k24, k93, k31, k37, k30, k32, k34, k33, k47, k40, k42, k44, k07, k46, k41, k43, k45, k94, k63, k56, k51, k53, k55, k71, k57, k50, k52, k54, k76, k67, k60, k62, k64, k95, ka6, k77, k70, k72, k74, k92, ka7, k87, k80, k82, k85, ka3, k86, k81, k83, k04, ka5, ul1, ur1, k97, ul2, ur2, k65, ul2, ur2, k15, ul3, ur3, k66, k05, ul3, ur3, k75, ul4, ur4, ka1, k25, ul5, ur5, k61, k91, ul6, ur6, ka2, k35, k03, ka4, k73"
l = "\
ul1,   k13, k26, k36, k31, k33, k07, k63, k71, k76, ka6, ka7, ka3, ka5, k97,     k01,   ur1, \n\
ul2,   k16, k17, k27, k37, k47, k46, k56, k57, k67, k77, k87, k86, k66, ka1,     k65,   ur2, \n \
ul3,   k11, k10, k20, k30, k40, k41, k51, k50, k60, k70, k80, k81, k61, ka2,     k15,   ur3, \n\
ul4,   k21, k12, k22, k32, k42, k43, k53, k52, k62, k72, k82, k83,     ka4,      k25,   ur4, \n \
ul5,   k00, k14, k24, k34, k44, k45, k55, k54, k64, k74, k85,    k91,       k35, k75,   ur5, \n\
ul6,   k06,    k90,    k93,       k94,          k95,     k92, k04,     k03, k73, k05,   ur6 "



leds = [x.strip() for x in l.split(",")]
s = [x.strip() for x in s.split(",")]
print(len(s))
print(len(leds))
for led in leds:
    try:
        i = s.index(led)
    except ValueError:
        i = 255
    l = l.replace(led, f"{i: 3}")

print(l)
f = open("res.txt", "w")
f.write(l)
f.close()