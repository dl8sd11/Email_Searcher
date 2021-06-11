a = "there are at least 73 members of the plantain order plantaginales found in montana 1 some of these species are exotics not native to montana 2 and some species have been designated as species of concern 3 family plantaginaceae i read the paragraph on http wikipedia org"

b = "minas rio is an iron ore mining project in brazil 1 it is one of the world s largest mining projects and is initially expected to export 26 5 million tonnes of iron ore in 2013 through a 525km slurry pipeline to a port at au 2 production potential is 53 mtpa or higher 3 the project was bought by anglo american plc which is facing high costs 4 5 the mine has certified reserves of 4 6 billion tonnes of itabirite 6 there have been delays in starting the project but in december 2010 anglo american obtained a key license needed from brazilian government before mining could start 7 8 i read the paragraph on http wikipedia org"

a = set(a.split(' '))
b = set(b.split(' '))

print(len(a&b))
print(len(a|b))
