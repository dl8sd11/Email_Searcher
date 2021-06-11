#include "api.h"
#include <stdbool.h>
#include <assert.h>
#include "src/io.h"
#include "src/pick.h"
#include "src/match.h"
#include "src/group.h"
#include "src/similar.h"
#include "src/token_parser.h"
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
