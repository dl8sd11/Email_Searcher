#include "api.h"
#include <stdbool.h>
#include <assert.h>
#include "src/io.h"
#include "src/pick.h"
#include "src/similar.h"
#include "src/token_parser.h"
#define HASH_TYPE long long

int main(void) {
	Data data;
  Ans ans;
	api.init(&data.n_mails, &data.n_queries, &data.mails, &data.queries);
  qsort(data.mails, data.n_mails, sizeof(data.mails), mailIdComp);
  TokenHash* mail_hash = mail_parser(&data);
  // step 1 : remap mail_hash.hash
  int allToken = 0;
  for (int i = 0; i < data.n_mails; i++){
    allToken += mailhash.len[i];
  }
  HASH_TYPE allTokenHash[allToken];
  int id = 0;
  for (int i = 0; i < data.n_mails; i++){
    for (int j = 0; j < mail_hash.len[i]; j++)
      allTokenHash[id++] = mail_hash.hash[i][j];
  }
  qsort(allTokenHash, allToken, sizeof(HASH_TYPE), hashComp);
  int uniqueToken = 0;
  uniqueHash(allTokenHash, allToken, &uniqueToken);
  for (int i = 0; i < data.n_mails; i++){
    for (int j = 0; j < mail_hash.len[i]; j++)
      mail_hash.hash[i][j] = SearchArrayId(allTokenHash, uniqueToken, mail_hash.hash[i][j]);
  }
  // *allTokenHash : array storing unique hashes
  // uniqueToken : length of the allTokenHash
  // step 1 end

  PickOrder pick_order[data.n_queries];
  SimilarGroup *similarGroups = pickSimilar(&data);

  int n_mails = data.n_mails;
  for (int i=0; i<n_mails; i++) {
    if (similarGroups[i].qSz == 0) continue;
    querySimilar(&data, &similarGroups[i], mail_hash);
  }

  return 0;
}
