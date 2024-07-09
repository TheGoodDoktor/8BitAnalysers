# This script converts a slightly modified version of the 6128 ROM disassembly from
# https://cpctech.cpc-live.com/docs/os.asm into a skool file.
# It could potentially be adapted to convert other asm files.

import string

NUM_FW_FUNCS = 201

# Count how many bytes a defb or defw statement represent.
def countDataBytes(prefix, line):
	if prefix == "defw":
		if '"' in line or ',' in line:
			print("ERROR: defw arrays are not implemented!")
		# we assume a defw is always a single item
		return 2

	# Get the text for the items, stripping off the comment text.	
	# This will fail if the items contain a ";" character
	arrayItems = line[4:].split(';')[0]
	
	# Split the text into a list, where a comma is the seperator.
	# This logic will break if any of the items contain a "," character
	itemList = arrayItems.split(',')  
	
	byteCount = 0
	for item in itemList:
		if '"' in item:
			firstQuote = item.find('"')
			lastQuote = item.rfind('"')
			text = item[firstQuote+1:lastQuote]
			byteCount = byteCount + len(text)
		else:
			byteCount = byteCount + 1
	return byteCount

# Dictionary of addresses -> labels
# If you want a label at a particular address you can add items to this manually.
addressLabels = {}

foundFuncs = False
numFuncsFound = 0

# Initial pass of file to gather firmware function names and addresses.
# These are stored as an array of words.
# Here is a sample entry: 
#
# defw &1b5c		;; 0 firmware function: KM INITIALISE
#
# Note: entries 46 & 47 point to the same address.

fr = open("6128ROM.asm", 'r')
for line in fr:

	if foundFuncs == False:
		if line[:4] == '08de':
			foundFuncs = True
	else:
		if numFuncsFound == NUM_FW_FUNCS:
			break
		fwFuncAddr = int(line[6:10], 16)
		fwFuncName = line[line.find(':')+2:-1].strip()
		fwFuncName = fwFuncName.replace(' ', '_')
		addressLabels[fwFuncAddr] = fwFuncName
		
		numFuncsFound = numFuncsFound + 1

fr.seek(0)
fw = open("6128ROM.skool", 'w')

curDirective = ' '
lastDirective = ' '
numLinesRead = 0
numLinesWritten = 0
curDataAddr = 0
funcsFound = 0

for line in fr:
		numLinesRead = numLinesRead + 1
		if line.isspace() or line[0] == '-':
				# skip blank lines or separators
				continue
		writeLine = True
		isComment = False
		if line[0] == ';':
				isComment = True
		elif ';' in line and line[:5].isspace():
				isComment = True

		if isComment ==  False:
				first4Chars = line[:4]
				first4CharsAreAddress = all(c in string.hexdigits for c in first4Chars)
				addrToAdd = None

				if first4Chars == "defb" or first4Chars == "defw":
						# This line is data. Note: this logic will fail if we have code at address $defb
						if line[5] == '"':
							curDirective = 't'
							line = line.replace('defb', 'defm')
						else:
							curDirective = 'b'
						first4CharsAreAddress = False
						addrToAdd = '$' + format(curDataAddr, '04x') + ' '
						curDataAddr = curDataAddr + countDataBytes(first4Chars, line)
				elif len(line) < 7 and first4CharsAreAddress:
						# This line contains an address and nothing else. These are always followed by data.
						curDataAddr = int(first4Chars, 16)
						writeLine = False
				elif first4CharsAreAddress and line[4:5] == ' ' and all(c in string.hexdigits for c in line[5:7]):
						# This line is code. Code lines start with "NNNN OO", where NNNN is the address and OO is the first byte of the opcode for the instruction.
						curDirective = 'c'
						
						# Is this one of the fw functions? If so, add a label.
						curAddr = int(first4Chars, 16)
						if curAddr in addressLabels:
							fw.write("@label=" + addressLabels[curAddr] + '\n')
							funcsFound = funcsFound + 1
							print('Found {0}'.format(addressLabels[curAddr]))
				else:
						print('Parse error at line {0}: {1}'.format(numLinesRead, line), end='')
				if writeLine:
						if curDirective != lastDirective:
								fw.write(curDirective)
						else:
								fw.write(' ')
						if addrToAdd != None:
								fw.write(addrToAdd)
						if first4CharsAreAddress:
								fw.write('$')

				lastDirective = curDirective
		else:
				writeLine = True

		if writeLine:
				fw.write(line)
				numLinesWritten = numLinesWritten + 1

fr.close()
fw.close()

print('Found {0}/{1} functions'.format(funcsFound, NUM_FW_FUNCS))
print('Done.\nRead {0} lines from in file.\nWrote {1} lines to out file'.format(numLinesRead, numLinesWritten))

