#include "api.h"
#include <stdbool.h>
#include <assert.h>

// pppppp
#define HASH_TYPE long long
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
    HASH_TYPE** hash; // hash[i][j] repr jth token of ith mail
    int* len;   // len[i] repr number of token of ith mail
} TokenHash;
typedef struct pick_order{
    int id;// ith query
    int time; //time per reward
}PickOrder;

typedef struct similar_group {
  int mId, qSz;
  double score;
  int *qIds, cap;
} SimilarGroup;

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

#define HASH_TYPE unsigned long long
const HASH_TYPE mod = (1ull<<61) - 1;
HASH_TYPE modmul(HASH_TYPE a, HASH_TYPE b){
	HASH_TYPE l1 = (int)a, h1 = a>>32, l2 = (int)b, h2 = b>>32;
	HASH_TYPE l = l1*l2, m = l1*h2 + l2*h1, h = h1*h2;
	HASH_TYPE ret = (l&mod) + (l>>61) + (h << 3) + (m >> 29) + (m << 35 >> 3) + 1;
	ret = (ret & mod) + (ret>>61);
	ret = (ret & mod) + (ret>>61);
	return ret-1;
}

HASH_TYPE hash3 (char *str) {
  HASH_TYPE base = 1;
  const HASH_TYPE c = 880301;
  HASH_TYPE sum = 0;
  while (*str) {
    base = modmul(base, c);
    HASH_TYPE cur = modmul(base, *str);
    sum = sum + cur >= mod ? sum + cur - mod : sum + cur;
    str++;
  }
  return sum;
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

#include <string.h>

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
            int sortedI = r, timecomp;
            if (sortedI == 0)
                timecomp = token*(log_prefix[mailN-1]-log_prefix[sortedI]);
            else
                timecomp = prefix[sortedI-1]*log(token) + token*(log_prefix[mailN-1]-log_prefix[sortedI]);
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

void initGroup (SimilarGroup* group, int mid) {
  group->mId = mid;
  group->qSz = 0;
  group->qIds = (int*)malloc(sizeof(int) * 16);
  group->cap = 16;
  group->score = 0;
}

void push_back (int **array, int item, int* cap, int *sz) {
  if (*sz == *cap) {
    (*cap) *= 2;
    *array = (int*)realloc(*array, (*cap)*sizeof(int));
  }
  (*array)[(*sz)++] = item;
}

int groupComp (const void *a, const void *b) {
  return ((SimilarGroup*)a)->score > ((SimilarGroup*)b)->score ? -1 : 1;
}

SimilarGroup* pickSimilar (Data *data) {
  int n_mails = data->n_mails, n_queries = data->n_queries;
  SimilarGroup* groups = (SimilarGroup*)malloc(sizeof(SimilarGroup) * n_mails);

  for (int i=0; i<n_mails; i++) {
    initGroup(groups + i, i);
  }

  for (int i=0; i<n_queries; i++) {
    if (data->queries[i].type != find_similar) continue;
    int mid = data->queries[i].data.find_similar_data.mid;
    int qid = data->queries[i].id;

    push_back(&groups[mid].qIds, qid, &groups[mid].cap, &groups[mid].qSz);
    groups[mid].score += data->queries[i].reward;
  }

  qsort(groups, n_mails, sizeof(SimilarGroup), groupComp);
  return groups;
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

    //printf("%s %d %d\n", token, token_hash, L != -1 && mail_hash[L] == token_hash);
    return L != -1 && mail_hash[L] == token_hash; 
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
	int* output = (int*)malloc(sizeof(int)*2);
	bool parts[1000];
	int boolidx = 0;
	char token[100];
	int tokenidx = 0;
	bool mult = false;
	bool reverse = false;
	bool bv;
	int i = stidx;
	while (input[i] != 0) {
		//printf("at: %c\n", input[i]);
		if (inToken(input[i])) {
			//printf("in token\n");
			token[tokenidx] = input[i];
			tokenidx++;
			i++;
		} else if (isOperator(input[i])) {
			//printf("bv : %d\n", bv);
			if (reverse) {
				bv = !bv;
				reverse = false;
			}
			if (input[i] == '|') {
				if (mult) {
					parts[boolidx] = parts[boolidx] && bv;
					mult = false;
					boolidx++;
				} else {
					parts[boolidx] = bv;
					boolidx++;
				}
			} else {
				if (mult) parts[boolidx] = parts[boolidx] && bv;
				else {
					parts[boolidx] = bv;
					mult = true;
				}
			}
			i++;
		} else if (isNot(input[i])) {
			reverse = true;
			i++;
		} else if (isParenthese(input[i])) {
			if (input[i] == '(') {
				int* unlock;
				unlock = Parse(i+1, input, mail_hash, mail_len);
				//printf("end recursion here\n");
				i = unlock[0];
				bv = (bool)unlock[1];
				if (reverse) {
					bv = !bv;
					reverse = false;
				}
				if (mult) parts[boolidx] = parts[boolidx] && bv;
				else parts[boolidx] = bv;
				if (isOperator(input[i])) {
					// printf("bv: %d opr: %c\n", bv, input[i]);
					if (input[i] == '|') {
						if (mult) {
							boolidx++;
							mult = false;
						} else {
							boolidx++;
						}
					} else {
						if (!mult)
							mult = true;
					}
					i++;
				}
				if (input[i] == ')') {
					i++;
					break;
				}
			} else {
				i++;
				break;
			}
		}
	}
	if (tokenidx != 0) {
		token[tokenidx] = '\0';
		if (reverse) bv = !check(token, mail_hash, mail_len);
		else bv = check(token, mail_hash, mail_len);
		if (mult) parts[boolidx] = parts[boolidx] && bv;
		else parts[boolidx] = bv;
	}
	int k = 0;
	for (int j=0; j<= boolidx; j++) {
		k += (bool)parts[j];
	}
	printf("(exprs) returns %d\n", k);
	int ck = (bool)k;
	if (reverse) ck = !ck;
	k = (int)ck;
	output[0] = i;
	output[1] = k;
	return output;
}

bool expression_parser(char* expression, int* mail_hash, int mail_len) {
  /*
  puts("============");
  for (int i=0; i<mail_len; i++) {
    printf("%d ", mail_hash[i]);
  }
  puts("============");
  */
	int* parseResult = Parse(0, expression, mail_hash, mail_len);
	bool ans = (bool)parseResult[1];
//  printf("%s %d\n", expression, ans);

	free(parseResult);
	return ans;
}

#include <string.h>
#define HASH_TYPE long long
HASH_TYPE* token_parser(char* content, int* mail_token_len);
//int* token_parser_hash(char *content);
TokenHash* mail_parser (Data* data);
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define HASH_TYPE long long
int compHash(const void *a, const void *b) {
    HASH_TYPE A = *(HASH_TYPE *)a, B = *(HASH_TYPE *)b;
    if (A > B) return 1;
    if (A < B) return -1;
    return 0;
}

void uniqueHash (HASH_TYPE* hashes, int len, int* resN) {
    for (int i=0, st=0; i<len; i++) {
        if (i == len-1 || hashes[i+1] != hashes[st]) {
            hashes[(*resN)++] = hashes[st];
            st = i + 1;
        }
    }
}

bool valid_char(char k) {								//check if character in [A-Za-z0-9]
	if (k > 47 && k < 58) return true;
	if (k > 64 && k < 91) return true;
	if (k > 96 && k < 123) return true;
	return false;
}

char lower (char c) {
  if (c >= 'A' && c <= 'Z') return c - 'A' + 'a';
  return c;
}
HASH_TYPE* token_parser (char* content, int* mail_token_len) {
  int N = strlen(content);
  (*mail_token_len) = 0;
  for (int i=0; i<N; i++) {
    if (valid_char(content[i]) && (i == 0 || !valid_char(content[i-1]))) {
      (*mail_token_len)++;
    }
  }

	HASH_TYPE* substringSet = (HASH_TYPE*)malloc(sizeof(HASH_TYPE*)*(*mail_token_len));		//the set of all sliced token
  int substringIdx = 0;
  char buf[100];

  int bid = 0;
  for (int i=0; i<N + 1; i++) {
    if (i < N && valid_char(content[i])) {
      buf[bid++] = lower(content[i]);
    } 
    if ((i == N || !valid_char(content[i])) && (i && valid_char(content[i-1]))) {
      char* cur = (char*)malloc(sizeof(char) * (bid + 1));
      buf[bid] = 0;
      strcpy(cur, buf);
      substringSet[substringIdx++] = hash3(cur);
      bid = 0;
    }
  }

  return substringSet;
}

char** token_parser_clone(char* content, int* mail_token_len) {
	int originContentIdx = 0;							//index of the content
	int substringIdx = 0;								//index of each substring(reset to 0 when start another one)	
	char** substringSet = (char**)malloc(sizeof(char*)*50);		//the set of all sliced token
	char* substring = (char*)malloc(sizeof(char)*4);			//the first token
	int substringMaxLen = 4;							//initalize the size of token
	int substringSetIdx = 0;							//index of the token set
	int substringSetSize = 50;							//initalize the size of token set

	while (content[originContentIdx] != 0) {			//start to read the content
		char key = content[originContentIdx];			//key = current character
		bool cut = false;								//cut shows the state 1.in a token -> false 2.out of a token -> true
		if (cut) {										//if currently out of a token
			if (valid_char(key)) {						//if valid character read
				cut = false;							//turn cut off
				substring = (char*)malloc(sizeof(char)*4);		//allocate a new space for token
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
					substring = (char*)realloc(substring, sizeof(char)*substringMaxLen);	//allocate it into double quota
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

        // fprintf(stderr, "put %p\n", substring);
				substringSet[substringSetIdx] = substring;	//put the token into the token set
				substringSetIdx++;						//move to the next index of token set
				if (!(substringSetIdx < substringSetSize-1)) {	//if the token set is almost full
					substringSetSize *= 2;				//resize the token set
					substringSet = (char**)realloc(substringSet, sizeof(char*)*substringSetSize);	//allocate the token set into double quota
				}
				originContentIdx++;						//move to next character(key)
			}
		}

		if (!cut) {										//if content end with a valid character (still in a token)
			substring[substringIdx] = '\0';				//put end of string in the token (cut the token)
      // fprintf(stderr, "put %p\n", substring);
			substringSet[substringIdx] = substring;	//put the token into the token set
		}
    substringSetIdx++;						//move to the next index of token set
    if (!(substringSetIdx < substringSetSize-1)) {	//if the token set is almost full
      substringSetSize *= 2;				//resize the token set
      substringSet = (char**)realloc(substringSet, sizeof(char*)*substringSetSize);	//allocate the token set into double quota
    }
	}
	*(mail_token_len) = substringSetIdx;
	return substringSet;							//return the token set
}

TokenHash* mail_parser(Data* data) {
	TokenHash* mail_hashes = (TokenHash*)malloc(sizeof(TokenHash));
	mail_hashes->hash = (HASH_TYPE**)malloc(sizeof(HASH_TYPE*)*data->n_mails);
	mail_hashes->len = (int*)malloc(sizeof(int)*data->n_mails);
	int* mail_len = (int*) malloc(sizeof(int));
  char c[100305];
	for (int i=0; i<data->n_mails; i++) {
    c[0] = 0;
    strcat(c, data->mails[i].content);
    strcat(c, data->mails[i].subject);

		HASH_TYPE* token_hashes = token_parser(c, mail_len);
		qsort(token_hashes, *(mail_len), sizeof(HASH_TYPE), compHash);

		int unq_len = 0;
		uniqueHash(token_hashes, *(mail_len), &unq_len);
		mail_hashes->hash[i] = token_hashes;
		mail_hashes->len[i] = unq_len;

	}
	free(mail_len);
	return mail_hashes;
}

void queryMatch (TokenHash* mail_hash, Data *data, char *expr, Ans *ans) {
    int n = data->n_mails;
    static int array[10004];
    ans->array = array;

    ans->len = 0;

    int m = strlen(expr);
    for (int i=0; i<m; i++) {
      expr[i] = lower(expr[i]);
    }

    for (int i=0; i<n; i++) {
        if (expression_parser(expr, mail_hash->hash[i], mail_hash->len[i])) {
            ans->array[ans->len++] = data->mails[i].id;
        }
    }
    qsort(ans->array, ans->len, sizeof(int), comp);
}

// this is only the testing version of api.h
// the input format and hashing function will be
// different on DSA Judge

#ifndef API_H
#define API_H
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

typedef struct mail {
	int id;
	char from[32], to[32];
	char subject[256], content[100000];
} mail;

typedef struct query {
	int id;
	double reward;

	enum query_type {
		expression_match,
		find_similar,
		group_analyse
	} type;

	union query_data {
		struct {
			char expression[2048];
		} expression_match_data;

		struct {
			int mid;
			double threshold;
		} find_similar_data;

		struct {
			int len, mids[512];
		} group_analyse_data;
	} data;
} query;

long _get_process_cputime(){
  static struct timespec t;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t);
  return t.tv_sec * 1000 + t.tv_nsec / 1000000;
}

void _assert(bool condition, char message[]){
	if(!condition){
		printf("assertion failed: %s", message);
		dup(0);  // calling a forbidden syscall to trigger Security Error on DSA Judge
		exit(1);
	}
}

void _answer(int qid, int answers[], int len){
	static bool initialized = false;
	static long _begin = 0;

	if(initialized == false){
		initialized = true;
		_begin = _get_process_cputime();
		return;
	}

	if(_get_process_cputime() - 5000 > _begin)
		exit(0);

	unsigned long long digest = 0x76efa04b78375b4b;
	for(int i = 0; i < len; i++){
		digest = (digest >> 17) | (digest << (64 - 17));
    	digest ^= (unsigned long long)answers[i];
    	digest = ~digest;
	}
	printf("%d %llx\n", qid, digest);
}

void api_init(int *n_mails, int *n_queries, mail **mails, query **queries){
	_assert(_get_process_cputime() < 15, "Calling init too late");

	// reading mails
	char buffer[102400];
	fgets(buffer, 1024, stdin);
	sscanf(buffer, "%d", n_mails);

	*mails = (mail *)malloc(*n_mails * sizeof(mail));

	fgets(buffer, 1024, stdin);
	_assert(strcmp(buffer, "=== dc685a9c7684059f ===\n") == 0, "Wrong input format");

	for(int i = 0; i < *n_mails; i++){
		mail *m = &(*mails)[i];

		fgets(buffer, 1024, stdin);
		sscanf(buffer, "%d", &(m->id));

		fgets(m->from, 32, stdin);
		m->from[strlen(m->from) - 1] = '\0';

		fgets(m->content, 100000, stdin);
		m->content[strlen(m->content) - 1] = '\0';

		fgets(m->subject, 256, stdin);
		m->subject[strlen(m->subject) - 1] = '\0';

		fgets(m->to, 32, stdin);
		m->to[strlen(m->to) - 1] = '\0';

		fgets(buffer, 1024, stdin);
		_assert(strcmp(buffer, "=== e47bbae5876702dc ===\n") == 0, "Wrong input format");
	}

	// reading queries
	fgets(buffer, 1024, stdin);
	sscanf(buffer, "%d", n_queries);

	*queries = (query *)malloc(*n_queries * sizeof(query));

	fgets(buffer, 1024, stdin);
	_assert(strcmp(buffer, "=== dc685a9c7684059f ===\n") == 0, "Wrong input format");

	for(int i = 0; i < *n_queries; i++){
		query *q = &(*queries)[i];

		int len;
		unsigned long long seed;

		fgets(buffer, 1024, stdin);
		sscanf(buffer, "%d %lf", &q->id, &q->reward);

		fgets(buffer, 1024, stdin);
		switch(buffer[0]){
			case 'e':
				q->type = expression_match;
				fgets(q->data.expression_match_data.expression, 2048, stdin);
				q->data.expression_match_data.expression[strlen(q->data.expression_match_data.expression) - 1] = '\0';
				break;
			case 'f':
				q->type = find_similar;
				fgets(buffer, 1024, stdin);
				sscanf(buffer, "%d %lf", &q->data.find_similar_data.mid, &q->data.find_similar_data.threshold);
				break;
			case 'g':
				q->type = group_analyse;
				fgets(buffer, 1024, stdin);
				sscanf(buffer, "%llu %d", &seed, &len);

				for(int i = 0; i < len; i++){
					seed = (seed * 48271LL) % 2147483647;
					q->data.group_analyse_data.mids[i] = seed % (*n_mails);
				}

				q->data.group_analyse_data.len = len;

				break;
		}

		fgets(buffer, 1024, stdin);
		_assert(strcmp(buffer, "=== e47bbae5876702dc ===\n") == 0, "Wrong input format");
	}

	_answer(0, 0, 0);
}

struct {
	void (*init)(int *n_mails, int *n_queries, mail **mails, query **queries);
	void (*answer)(int qid, int answers[], int len);
} api = {api_init, _answer};

#endif

#ifndef DBG
#define fprintf(...)
#endif
#define HASH_TYPE long long

int hashInSortedArray (HASH_TYPE hash, HASH_TYPE* array, int sz) {
  int L = -1, R = sz;
  while (L < R - 1) {
    int M = (L + R) >> 1;
    if (array[M] < hash) L = M;
    else R = M;
  }
  return R < sz && array[R] == hash;
}

int calcIntersection (int x, int y, TokenHash* mail_hash) {
  int res = 0;
  if (mail_hash->len[x] > mail_hash->len[y]) {
    int tmp = x;
    x = y;
    y = tmp;
  }

  for (int i=0; i<mail_hash->len[x]; i++) {
    res += hashInSortedArray(mail_hash->hash[x][i], mail_hash->hash[y], mail_hash->len[y]);
  }

  return res;
}

typedef struct similar_data {
  int id;
  double jaccard;
} SimilarData;

int jacComp (const void *a, const void *b) {
  return ((SimilarData*)b)->jaccard - ((SimilarData*)a)->jaccard > 0 ? 1 : -1;
}

void querySimilar (Data *data, SimilarGroup* group, TokenHash* mail_hash) {
  fprintf(stderr, "mail %d, %d queries\n", group->mId, group->qSz);
  int target = group->mId;
  
  static int intersection[10004];
  static SimilarData res[10004];

  for (int i=0; i<data->n_mails; i++) {
    if (i != target) intersection[i] = calcIntersection(i, target, mail_hash);
    else intersection[i] = -1;

    res[i].id = i;
    res[i].jaccard = intersection[i] / (double)(mail_hash->len[i] + mail_hash->len[target] - intersection[i]);
  }

  fprintf(stderr, "%d %lf\n", data->n_mails, res[869].jaccard);
  for (int i=0; i<100; i++) {
    fprintf(stderr, "%lf ", res[i].jaccard);
  }
  fprintf(stderr, "\n");
  qsort(res, data->n_mails, sizeof(SimilarData), jacComp);

  static int ans[10004];
  for (int i=0; i<group->qSz; i++) {
    int qid = group->qIds[i];
    double threshold = data->queries[qid].data.find_similar_data.threshold;
    fprintf(stderr, "qid: %d %lf\n", qid, threshold);
    int asz = 0;

    for (int j=0; res[j].jaccard > threshold; j++) {
      ans[asz++] = res[j].id;
    }
    qsort(ans, asz, sizeof(int), comp);

    for (int j=0; j<asz; j++) {
      fprintf(stderr, "%d ", ans[j]);
    }
    fprintf(stderr, "\n");
    api.answer(qid, ans, asz);
  }
}

#define HASH_TYPE long long

int mailCompID (const void *a, const void *b) {
  return ((mail*)a)->id - ((mail*)b)->id;
}

void sortMail (mail* mails, int sz) {
  qsort(mails, sz, sizeof(mail), mailCompID);
}

int main(void) {
	Data data;
  Ans ans;
	api.init(&data.n_mails, &data.n_queries, &data.mails, &data.queries);
  sortMail(data.mails, data.n_mails);
  TokenHash* mail_hash = mail_parser(&data);

  PickOrder pick_order[data.n_queries];
  SimilarGroup *similarGroups = pickSimilar(&data);

  int n_mails = data.n_mails;
  for (int i=0; i<n_mails; i++) {
    if (similarGroups[i].qSz == 0) continue;
    querySimilar(&data, &similarGroups[i], mail_hash);
  }

  return 0;
}