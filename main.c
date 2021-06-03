#include "api.h"
#include "stdbool.h"
#include "src/io.h"
#include "src/pick.h"
#include "src/match.h"
#include "src/group.h"
#include "src/similar.h"
#include "src/token_parser.h"

int main(void) {
	Data data;
    Ans ans;
	api.init(&data.n_mails, &data.n_queries, &data.mails, &data.queries);
    TokenHash* mail_hash = NULL;//mail_parser(&data);


    int picked = 0;
	while (true) {
        int pid = pickProblem(&data, &picked);
        if (pid < 0) break;

        if (data.queries[pid].type == expression_match) {
            queryMatch(mail_hash, &data, data.queries[pid].data.expression_match_data.expression, &ans);
            api.answer(data.queries[pid].id, ans.array, ans.len);
        } else if (data.queries[pid].type == find_similar) {
            querySimilar(&data, data.queries[pid].data.find_similar_data.mid, data.queries[pid].data.find_similar_data.threshold, &ans);
            api.answer(data.queries[pid].id, ans.array, ans.len);
        } else if (data.queries[pid].type == group_analyse) {
            int len = data.queries[pid].data.group_analyse_data.len; 
            int* mids = data.queries[pid].data.group_analyse_data.mids; 
            queryGroup(&data, &ans, len, mids);
            api.answer(data.queries[pid].id, ans.array, ans.len);
        } 
	}
    return 0;
}
