#include "api.h"
#include <stdbool.h>
#include <assert.h>

// pppppp
typedef struct data {
    int n_mails, n_queries;
    mail *mails;
    query *queries;
} Data;

typedef struct ans {
    int* array;
    int len;
} Ans;

typedef struct token_hash {
    int** hash; // hash[i][j] repr jth token of ith mail
    int* len;   // len[i] repr number of token of ith mail
} TokenHash;
typedef struct pick_order{
    int id;// ith query
    int time; //time per reward
}PickOrder;

#include <math.h>

#include <stdlib.h>

int hash1(char *str);
int hash2(char *str);
#include <stdio.h>
#include <string.h>
int hash1(char *str){// bkdr
    unsigned int seed = 131; 
    unsigned int hash = 0;
    while (*str){
        hash = hash * seed + (*str++);
    }
    return (int)hash;
}
int hash2(char *str){// djb2
    unsigned int hash = 5381;
    while (*str){   
        hash = ((hash << 5) + hash) + (*str++);
    }
    return (int)hash;
}

int comp(const void *a, const void *b) {
    int A = *(int *)a, B = *(int *)b;
    if (A > B) return 1;
    if (A < B) return -1;
    return 0;
}

void unique (int* hashes, int len, int* resN) {
    for (int i=0, st=0; i<len; i++) {
        if (i == len-1 || hashes[i+1] != hashes[st]) {
            hashes[(*resN)++] = hashes[st];
            st = i + 1;
        }
    }
}

int getId (char *name, int* hashes, int hashN) {
    int nameHash = hash1(name);

    // lower_bound(nameHash)
    int L = -1, R = hashN;
    while (L < R - 1) {
        int M = (L + R) >> 1;
        if (hashes[M] >= nameHash) R = M;
        else L = M;
    }
    return R;
}

int fnd (int x, int *djs) {
    if (x == djs[x]) return x;
    djs[x] = fnd(djs[x], djs);
    return djs[x];
}

void merge (int a, int b, int *djs, int *sz) {
    a = fnd(a, djs);
    b = fnd(b, djs);
    if (a == b) return;
    if (sz[a] > sz[b]) {
        int tmp = a;
        a = b;
        b = tmp;
    }

    djs[a] = b;
    sz[b] += sz[a];
}

void queryGroup (Data *data, Ans *ans, int len, int *mids) {

    int* hashes = (int*)malloc(len * 2 * sizeof(int));
    for (int i=0; i<len; i++) {
        int mid = mids[i];
        hashes[i<<1] = hash1(data->mails[mid].from);
        hashes[i<<1|1] = hash1(data->mails[mid].to);
        //fprintf(stderr, "%s %s\n", data->mails[mid].from, data->mails[mid].to);
    }

    qsort(hashes, len*2, sizeof(int), comp);
    int hashN = 0;
    unique(hashes, len*2, &hashN);

    int* djs = (int*)malloc(hashN * sizeof(int));
    int* sz = (int*)malloc(hashN * sizeof(int));
    for (int i=0; i<hashN; i++) {
        djs[i] = i;
        sz[i] = 1;
    }

    for (int i=0; i<len; i++) {
        int mid = mids[i];
        int fid = getId(data->mails[mid].from, hashes, hashN);
        int tid = getId(data->mails[mid].to, hashes, hashN);
        //fprintf(stderr, "%d %d\n", fid, tid);
        merge(fid, tid, djs, sz);
    }

    ans->len = 2;
    ans->array = (int*)malloc(2 * sizeof(int));
    ans->array[0] = 0;
    ans->array[1] = 0;

    for (int i=0; i<hashN; i++) {
        if (djs[i] == i && sz[i] > 1) {
            ans->array[0]++;
            //fprintf(stderr, "group %d %d\n", djs[i], sz[i]);
            if (sz[i] > ans->array[1]) {
                ans->array[1] = sz[i];
            }
        }
    }
    //fprintf(stderr, "%d %d\n",ans->array[0], ans->array[1]);
    free(djs);
    free(sz);
    free(hashes);

}

TokenHash* mail_parser (Data* data) {

}

void compute_prefix(int n_mails, int* mail_token, int* sorted, long long int* log_prefix, long long int* prefix){
    for (int i = 0; i < n_mails; i++)
        sorted[i] = mail_token[i];
    qsort(sorted, n_mails, sizeof(int), comp);

    log_prefix[0] = (int)log(mail_token[0]);
    prefix[0] = mail_token[0];
    for (int i = 1; i < n_mails; i++){
        log_prefix[i] = log_prefix[i-1] + (int)log(mail_token[i]);
        prefix[i] = prefix[i-1] + mail_token[i];
    }
}
int pickComp(const void *a, const void *b){
    PickOrder A = *(PickOrder *)a, B = *(PickOrder *)b;
    if (A.time > B.time) return 1;
    if (A.time < B.time) return -1;
    return 0;
}
void pickProblem(PickOrder *pick_order, TokenHash* mail_hash, Data* data){
    int mailN = data->n_mails, queryN = data->n_queries;
    for (int i = 0; i < queryN; i++)
        pick_order[i].id = i;
    long long int prefix[mailN];
    long long int log_prefix[mailN];
    int sorted[mailN];
    compute_prefix(mailN, mail_hash->len, sorted, log_prefix, prefix);
    for (int i = 0; i < queryN; i++){
        if (data->queries[i].type == expression_match){
            // (token of expression) * (log(token1)+log(token2)...)
            int expr_token = strlen((data->queries[i]).data.expression_match_data.expression) / 3;// bongi's estimate
            pick_order[i].time = expr_token * log_prefix[mailN-1] / (data->queries[i]).reward;
        }   
        if (data->queries[i].type == find_similar){
            //min(tokenA, tokeni) * log(max(tokenA, tokeni))
            // = (token1+...+tokenA-1) * log(tokenA) + tokenA * (log(tokenA+1)+...))
            int l = -1, r = mailN;
            int tokenI = (data->queries[i]).data.find_similar_data.mid;
            int token = mail_hash->len[tokenI];
            while(l < r-1){
                int m = (l+r) / 2;
                if (token <= sorted[m]) r = m;
                else l = m;
            }
            int sortedI = r;
            int timecomp = prefix[sortedI-1]*log(token) + token*(log_prefix[mailN-1]-log_prefix[sortedI]);
            pick_order[i].time = timecomp / (data->queries[i]).reward;
        }
        else{
            // len * log(len)
            int len = (data->queries[i]).data.group_analyse_data.len;
            pick_order[i].time = len*log(len) / (data->queries[i]).reward;
        }
    }
    qsort(pick_order, queryN, sizeof(PickOrder), pickComp);
    // time short -> time long
 
}

void pickOnly(PickOrder *order, Data *data, int tp) {
    int queryN = data->n_queries;
    int* res = (int*)malloc(queryN * sizeof(int));
    int resN = 0;
    for (int i=0; i<queryN; i++) {
        if (data->queries[i].type == tp) res[resN++] = i;
    }
    for (int i=0; i<queryN; i++) {
        if (data->queries[i].type != tp) res[resN++] = i;
    }
    for (int i=0; i<queryN; i++) order[i].id = res[i];
    free(res);
}

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

bool ckeck(char* token, int* mail_hash, int mail_len);
bool isParenthese(char key);
bool isOperator(char key);
bool isNot(char key);
bool inToken(char key);
int* Parse(int stidx, char* input, int* mail_hash, int mail_len);
bool expression_parser(char* expression, int* mail_hash, int mail_len);

bool check(char* token, int* mail_hash, int mail_len) {
    int token_hash = hash1(token);
    // mail_hash should be sorted

    int L = -1, R = mail_len;
    // lower_bound(mail_hash, token_hash)
    while (L < R - 1) {
        int M = (L + R) >> 1;
        if (mail_hash[M] <= token_hash) L = M;
        else R = M;
    }

    return mail_hash[L] == token_hash; 
}

bool inToken(char key) {
	if (key >= 'A' && key <= 'Z') return true;
	if (key >= 'a' && key <= 'z') return true;
	if (key >= '0' && key <= '9') return true;
	return false;
}

bool isParenthese(char key) {
	if (key == '(' || key == ')') return true;
	return false;
}

bool isNot(char key) {
	if (key == '!') return true;
	return false;
}

bool isOperator(char key) {
	if (key == '&' || key == '|') return true;
	return false;
}

int* Parse(int stidx, char* input, int* mail_hash, int mail_len) {
	int* output = malloc(sizeof(int)*2);
	bool parts[1000];
	int boolidx = 0;
	char token[100];
	int tokenidx = 0;
	bool and = false;
	bool not = false;
	int i = stidx;
	while (input[i] != 0) {
		if (inToken(input[i])) {
			token[tokenidx] = input[i];
			tokenidx++;
			i++;
		} else if (isOperator(input[i])) {
			token[tokenidx] = '\0';
			tokenidx = 0;
			bool bv = check(token, mail_hash, mail_len);
			if (not) {
				bv = !bv;
				not = false;
			}
			if (input[i] == '|') {
				if (and) {
					parts[boolidx] = parts[boolidx] && bv;
					and = false;
					boolidx++;
				} else {
					parts[boolidx] = bv;
					boolidx++;
				}
			} else {
				if (and) parts[boolidx] = parts[boolidx] && bv;
				else {
					parts[boolidx] = bv;
					and = true;
				}
			}
			i++;
		} else if (isNot(input[i])) {
			not = true;
		} else if (isParenthese(input[i])) {
			if (input[i] == '(') {
				int* unlock;
				unlock = Parse(i+1, input, mail_hash, mail_len);
				i = unlock[0];
				bool bv = (bool)unlock[1];
				if (not) {
					bv = !bv;
					not = false;
				}
				if (and) parts[boolidx] = parts[boolidx] && bv;
				else parts[boolidx] = bv;
				if (isOperator(input[i])) {
					if (input[i] == '|') {
						if (and) {
							boolidx++;
							and = false;
						} else {
							boolidx++;
						}
					} else {
						if (!and)
							and = true;
						}
					}
					i++;
			} else {
				i++;
				break;
			}
		}
	}
	if (tokenidx != 0) {
		token[tokenidx] = '\0';
		bool bv;
		if (not) bv = !check(token, mail_hash, mail_len);
		else bv = check(token, mail_hash, mail_len);
		if (and) parts[boolidx] = parts[boolidx] && bv;
		else parts[boolidx] = bv;
	}
	int k = 0;
	for (int j=0; j<= boolidx; j++) {
		k += (bool)parts[j];
	}
	output[0] = i;
	output[1] = k;
	return output;
}

bool expression_parser(char* expression, int* mail_hash, int mail_len) {
	int* parseResult = Parse(0, expression, mail_hash, mail_len);
	return (bool)parseResult[1];
}

void queryMatch (TokenHash* mail_hash, Data *data, char *expr, Ans *ans) {
    int n = data->n_mails;

    ans->len = 0;
    for (int i=0; i<n; i++) {
        if (expression_parser(expr, mail_hash->hash[i], mail_hash->len[i])) {
            ans->array[ans->len++] = data->mails[i].id;
        }
    }
    qsort(ans->array, ans->len, sizeof(int), comp);
}

void querySimilar (Data *data, int mid, double threshold, Ans *ans) {

}

char** token_parser(char* content);
int* token_parser_hash(char *content);
TokenHash* mail_parser (Data* data);
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

bool valid_char(char k) {								//check if character in [A-Za-z0-9]
	if (k > 47 && k < 58) return true;
	if (k > 64 && k < 91) return true;
	if (k > 96 && k < 123) return true;
	return false;
}

char** token_parser(char* content) {
	int originContentIdx = 0;							//index of the content
	int substringIdx = 0;								//index of each substring(reset to 0 when start another one)	
	char** substringSet = malloc(sizeof(char*)*50);		//the set of all sliced token
	char* substring = malloc(sizeof(char)*4);			//the first token
	int substringMaxLen = 4;							//initalize the size of token
	int substringSetIdx = 0;							//index of the token set
	int substringSetSize = 50;							//initalize the size of token set

	while (content[originContentIdx] != 0) {			//start to read the content
		char key = content[originContentIdx];			//key = current character
		bool cut = false;								//cut shows the state 1.in a token -> false 2.out of a token -> true
		if (cut) {										//if currently out of a token
			if (valid_char(key)) {						//if valid character read
				cut = false;							//turn cut off
				substring = malloc(sizeof(char)*4);		//allocate a new space for token
				substringIdx = 0;						//reset index inside token
				substring[substringIdx] = key;			//put key into the stored token
				substringIdx++;							//move to next index
				originContentIdx++;						//move to next character(key)
			} else {									//if key still invalid
				originContentIdx++;						//move to next character
			}
		} else {										//if currently in a token
			if (valid_char(key)) {						//if valid character read
				if (substringIdx < substringMaxLen-1) { //if the token is not about to be full
					substring[substringIdx] = key;		//fill in the key
					substringIdx++;						//move to next index
					originContentIdx++;					//move to next character(key)
				} else {								//if the token is almost full
					substringMaxLen *= 2;				//resize the token 
					substring = realloc(substring, sizeof(char)*substringMaxLen);	//allocate it into double quota
					substring[substringIdx] = key;		//fill in the key
					substringIdx++;						//move to next index
					originContentIdx++;					//move to next character(key)
				}
			} else {									//if an invalid key read
				if (substringIdx == 0 && substringSetIdx == 0) {	//handle if the first token not yet started
					originContentIdx++;
					continue;
				}
				cut = true;								//cut the token
				substring[substringIdx] = '\0';			//put end of string in the token
				substringMaxLen = 4;					//reset the token size
				substringIdx = 0;						//reset the token index
				substringSet[substringSetIdx] = substring;	//put the token into the token set
				substringSetIdx++;						//move to the next index of token set
				if (!(substringSetIdx < substringSetSize-1)) {	//if the token set is almost full
					substringSetSize *= 2;				//resize the token set
					substring = realloc(substring, sizeof(char*)*substringSetSize);	//allocate the token set into double quota
				}
				originContentIdx++;						//move to next character(key)
			}
		}

		if (!cut) {										//if content end with a valid character (still in a token)
			substring[substringIdx] = '\0';				//put end of string in the token (cut the token)
			substringSet[substringSetSize] = substring;	//put the token into the token set
		}
	}

	return substringSet;							//return the token set
}

int main(void) {
	Data data;
    Ans ans;
	api.init(&data.n_mails, &data.n_queries, &data.mails, &data.queries);
    TokenHash* mail_hash = NULL;//mail_parser(&data);

    PickOrder pick_order[data.n_queries];
    int pickI = 0;
    pickOnly(pick_order, &data, group_analyse);

    int cnt = 0;
	while (true) {
        int pid = pick_order[pickI++].id;
        if (data.queries[pid].type == expression_match) {
            break;
            queryMatch(mail_hash, &data, data.queries[pid].data.expression_match_data.expression, &ans);
            api.answer(data.queries[pid].id, ans.array, ans.len);
        } else if (data.queries[pid].type == find_similar) {
            break;
            querySimilar(&data, data.queries[pid].data.find_similar_data.mid, data.queries[pid].data.find_similar_data.threshold, &ans);
            api.answer(data.queries[pid].id, ans.array, ans.len);
        } else if (data.queries[pid].type == group_analyse) {
            int len = data.queries[pid].data.group_analyse_data.len; 
            int* mids = data.queries[pid].data.group_analyse_data.mids; 
//            if (data.queries[pid].id != 2442) continue;
            queryGroup(&data, &ans, len, mids);
            api.answer(data.queries[pid].id, ans.array, ans.len);
            free(ans.array);
        } 
	}
    return 0;
}