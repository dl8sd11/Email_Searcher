#include "api.h"
#include <stdbool.h>
#include <assert.h>
#include "src/io.h"
#include "src/pick.h"
#include "src/similar.h"
#include "src/token_parser.h"
#include "src/token_table_gen.h"
#define HASH_TYPE long long

int main(void) {
	Data data;
  Ans ans;
	api.init(&data.n_mails, &data.n_queries, &data.mails, &data.queries);

  qsort(data.mails, data.n_mails, sizeof(mail), mailIdComp);

  TokenHash* mail_hash = mail_parser(&data);

  int uniqueTokens = reorderMailHash(mail_hash, data.n_mails);
  TokenTable* token_table = genTokenTable(mail_hash, data.n_mails, uniqueTokens);

  PickOrder pick_order[data.n_queries];
  SimilarGroup *similarGroups = pickSimilar(&data);

  int n_mails = data.n_mails;
  for (int i=0; i<n_mails; i++) {
    if (similarGroups[i].qSz == 0) continue;
    querySimilar(&data, &similarGroups[i], mail_hash, token_table);
  }

  return 0;

}
