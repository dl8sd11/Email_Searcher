#include "api.h"
#include <stdbool.h>
#include <assert.h>
#include "src/io.h"
#include "src/pick.h"
#include "src/similar.h"
#include "src/token_parser.h"
#include "src/token_table_gen.h"
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
