 ## step 1

TokenHash: hash space from LONG_LONG to TOKEN_SIZE

- collects hashes 1
- unique hashes 1
- remap hashes 1 to hashes 2

## step 2

Create reverse table of TokenHash

TokenTable: 

int mailId[token_id][x] // mailId[token_id]: the array of mailId contains [token_id]

- dynamic 2d - array
- count the size before malloc

## step 3

Calculate similarity using TokenHash

- iterate through tokens
- increase the count on the matches


