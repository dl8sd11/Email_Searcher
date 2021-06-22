#include "api.h"
#include <stdbool.h>
#include <assert.h>

// pppppp
#define HASH_TYPE int
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

typedef struct token_table {
  int **mailId;
  int *len;
} TokenTable;

typedef struct pick_order{
  int id;// ith query
  int time; //time per reward
}PickOrder;

typedef struct similar_group {
  int mId, qSz;
  double score;
  int *qIds, cap;
} SimilarGroup;

typedef struct similar_data {
  int id;
  double jaccard;
} SimilarData;

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

#define HASH_TYPE int

#include <string.h>

bool valid_char(char k) {								//check if character in [A-Za-z0-9]
  if (k > 47 && k < 58) return true;
  if (k > 64 && k < 91) return true;
  if (k > 96 && k < 123) return true;
  return false;
}

int hashComp(const void *a, const void *b) {
  HASH_TYPE A = *(HASH_TYPE *)a, B = *(HASH_TYPE *)b;
  if (A > B) return 1;
  if (A < B) return -1;
  return 0;
}

int pickComp(const void *a, const void *b){
  PickOrder A = *(PickOrder *)a, B = *(PickOrder *)b;
  if (A.time > B.time) return 1;
  if (A.time < B.time) return -1;
  return 0;
}

int groupComp (const void *a, const void *b) {
  return ((SimilarGroup*)a)->score > ((SimilarGroup*)b)->score ? -1 : 1;
}

int jacComp (const void *a, const void *b) {
  return ((SimilarData*)b)->jaccard - ((SimilarData*)a)->jaccard > 0 ? 1 : -1;
}

int mailIdComp (const void *a, const void *b) {
  return ((mail*)a)->id - ((mail*)b)->id;
}

/*
unique hashes 
input: a sorted array of hashes
output: uniqued array in hashes[0..resN]
*/
void uniqueHash (HASH_TYPE* hashes, int len, int* resN) {
  for (int i=0, st=0; i<len; i++) {
    if (i == len-1 || hashes[i+1] != hashes[st]) {
      hashes[(*resN)++] = hashes[st];
      st = i + 1;
    }
  }
}

/*
binary search hash in array 
output: 0, 1
*/
int hashInSortedArray (HASH_TYPE hash, HASH_TYPE* array, int sz) {
  int L = -1, R = sz;
  while (L < R - 1) {
    int M = (L + R) >> 1;
    if (array[M] < hash) L = M;
    else R = M;
  }
  return R < sz && array[R] == hash;
}
/*
binary search array index
example: sorted array = [1,3,8]
         if target = 8, return 2
*/
int SearchArrayId(HASH_TYPE *array, int arrayN, HASH_TYPE target){
	int L = -1, R = arrayN, M;
	while (L < R-1){
		M = (L+R)/2;
		if (array[M] >= target) R = M;
		else L = M;
	}
	return R;
}

char lower (char c) {
  if (c >= 'A' && c <= 'Z') return c - 'A' + 'a';
  return c;
}

void push_back (int **array, int item, int* cap, int *sz) {
  if (*sz == *cap) {
  (*cap) *= 2;
  *array = (int*)realloc(*array, (*cap)*sizeof(int));
  }
  (*array)[(*sz)++] = item;
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
#define HASH_TYPE int

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

void querySimilarBinarySearch (Data *data, SimilarGroup* group, TokenHash* mail_hash) {
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

void querySimilar (Data *data, SimilarGroup* group, TokenHash* mail_hash, TokenTable* token_table) {
  // fprintf(stderr, "mail %d, %d queries\n", group->mId, group->qSz);
  int target = group->mId;
  
  static double res[10004];

  for (int i=0; i<data->n_mails; i++) res[i] = 0;
  for (int i=0; i<mail_hash->len[target]; i++) {
    int hash = mail_hash->hash[target][i];
    for (int j=0; j<token_table->len[hash]; j++) {
      int mid = token_table->mailId[hash][j];
      res[mid]++;
    }
  }
  res[target] = -1;

  for (int i=0; i<data->n_mails; i++) {
    res[i] = res[i] / (double)(mail_hash->len[i] + mail_hash->len[target] - res[i]);
  }

  // qsort(res, data->n_mails, sizeof(SimilarData), jacComp);

  static int ans[10004];
  for (int i=0; i<group->qSz; i++) {
    int qid = group->qIds[i];
    double threshold = data->queries[qid].data.find_similar_data.threshold;
    // fprintf(stderr, "qid: %d %lf\n", qid, threshold);
    int asz = 0;

    for (int j=0; j < data->n_mails; j++) {
      if (res[j] > threshold) {
        ans[asz++] = j;
      }
    }
    // qsort(ans, asz, sizeof(int), comp);
    api.answer(qid, ans, asz);
  }
}

#undef fprintf(...)

#include <string.h>
#define HASH_TYPE int
HASH_TYPE* token_parser(char* content, int* mail_token_len);
//int* token_parser_hash(char *content);
TokenHash* mail_parser (Data* data);
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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
      substringSet[substringIdx++] = hash1(cur);
      bid = 0;
    }
  }

  return substringSet;
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
    qsort(token_hashes, *(mail_len), sizeof(HASH_TYPE), hashComp);

    int unq_len = 0;
    uniqueHash(token_hashes, *(mail_len), &unq_len);
    mail_hashes->hash[i] = token_hashes;
    mail_hashes->len[i] = unq_len;

  }
  free(mail_len);
  return mail_hashes;
}

#include <stdio.h>
#include <stdlib.h>

int reorderMailHash (TokenHash* mail_hash, int n_mails) {
  // step 1 : remap mail_hash.hash
  int allToken = 0;
  for (int i = 0; i < n_mails; i++){
    allToken += mail_hash->len[i];
  }
  HASH_TYPE* allTokenHash = (HASH_TYPE*)malloc(sizeof(HASH_TYPE) * allToken);
  int id = 0;
  for (int i = 0; i < n_mails; i++){
    for (int j = 0; j < mail_hash->len[i]; j++)
      allTokenHash[id++] = mail_hash->hash[i][j];
  }
  qsort(allTokenHash, allToken, sizeof(HASH_TYPE), hashComp);
  int uniqueToken = 0;
  uniqueHash(allTokenHash, allToken, &uniqueToken);
  for (int i = 0; i < n_mails; i++){
    for (int j = 0; j < mail_hash->len[i]; j++)
      mail_hash->hash[i][j] = SearchArrayId(allTokenHash, uniqueToken, mail_hash->hash[i][j]);
  }

  free(allTokenHash);
  // *allTokenHash : array storing unique hashes
  // uniqueToken : length of the allTokenHash
  // step 1 end

  return uniqueToken;
}
     
TokenTable* genTokenTable(TokenHash* mail_hash, int mail_count, int hash_len) {
  int* precount = (int*) calloc(hash_len, sizeof(int));
  for (int i=0; i<mail_count; i++) {
    for (int j=0; j<mail_hash->len[i]; j++) {
      precount[(int)mail_hash->hash[i][j]]++;
    }
  }
  int** tokenRef = (int**) malloc(sizeof(int*)*hash_len);                                                                                                       
  TokenTable* result = (TokenTable*)malloc(sizeof(TokenTable));
  result->len = (int*)malloc(sizeof(int) * hash_len);

  result->mailId = tokenRef;
  for (int i=0; i<hash_len; i++) {
    if(precount[i]) { 
      tokenRef[i] = (int*) malloc(sizeof(int)*precount[i]);
    } else {
      tokenRef[i] = NULL;
    }
    result->len[i] = precount[i];
  }
  for (int i=0; i<mail_count; i++) {
    for (int j=0; j<mail_hash->len[i]; j++) {
      int num = (int) mail_hash->hash[i][j];
      tokenRef[num][precount[num]-1] = i;
      precount[num]--;
    }
  }
  free(precount);
  return result;
}

#define HASH_TYPE long long
clock_t prev = 0;

void start () {
  prev = clock();
}

#define timer(NAME) do{fprintf(stderr, #NAME);stop();}while(0);

void stop () {
  clock_t delta = clock() - prev;
  fprintf(stderr, " Time interval of : %lf\n", (double)(delta) / CLOCKS_PER_SEC);
}

int main(void) {
	Data data;
  Ans ans;
	api.init(&data.n_mails, &data.n_queries, &data.mails, &data.queries);

  qsort(data.mails, data.n_mails, sizeof(mail), mailIdComp);

  start();
  TokenHash* mail_hash = mail_parser(&data);
  timer("Mail Parser");

  start();
  int uniqueTokens = reorderMailHash(mail_hash, data.n_mails);
  timer("Reorder");
  TokenTable* token_table = genTokenTable(mail_hash, data.n_mails, uniqueTokens);

  PickOrder pick_order[data.n_queries];
  SimilarGroup *similarGroups = pickSimilar(&data);

  int n_mails = data.n_mails;
  for (int i=0; i<n_mails; i++) {
    if (similarGroups[i].qSz == 0) continue;
    querySimilar(&data, &similarGroups[i], mail_hash, token_table);
  }
  timer();

  return 0;
}