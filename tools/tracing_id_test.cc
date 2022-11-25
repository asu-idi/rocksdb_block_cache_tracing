//  Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).
//
// Copyright (c) 2012 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef ROCKSDB_LITE
#ifndef GFLAGS
#include <cstdio>
int main() {
  fprintf(stderr, "Please install gflags to run trace_analyzer test\n");
  return 0;
}
#else

#include "db/db_test_util.h"
#include "rocksdb/db.h"
#include "rocksdb/status.h"

namespace ROCKSDB_NAMESPACE {

class TracingIDTest : public testing::Test {
 public:
  TracingIDTest() {
    test_path_ = test::PerThreadDBPath("tracing_id_test");
    env_ = ROCKSDB_NAMESPACE::Env::Default();
    env_->CreateDir(test_path_).PermitUncheckedError();
    dbname_ = test_path_ + "/db";
  }

  ~TracingIDTest() override {}

  ROCKSDB_NAMESPACE::Env* env_;
  EnvOptions env_options_;
  std::string test_path_;
  std::string dbname_;
};

TEST_F(TracingIDTest, Iterator) {
  Options options;
  options.create_if_missing = true;
  options.merge_operator = MergeOperators::CreatePutOperator();
  //    Slice upper_bound("a");
  //    Slice lower_bound("abce");
  ReadOptions ro;
  //    ro.iterate_upper_bound = &upper_bound;
  //    ro.iterate_lower_bound = &lower_bound;
  WriteOptions wo;
  TraceOptions trace_opt;
  DB* db_ = nullptr;
  std::string value;
  std::unique_ptr<TraceWriter> trace_writer;
  Iterator* single_iter = nullptr;
  ASSERT_OK(DB::Open(options, dbname_, &db_));
  WriteBatch batch;
  ASSERT_OK(batch.Put("a", "aaaaaaaaa"));
  ASSERT_OK(batch.Merge("b", "aaaaaaaaaaaaaaaaaaaa"));
  ASSERT_OK(batch.Delete("c"));
  ASSERT_OK(batch.SingleDelete("d"));
  ASSERT_OK(batch.DeleteRange("e", "f"));
  ASSERT_OK(db_->Write(wo, &batch));
  std::vector<Slice> keys;
  keys.push_back("a");
  keys.push_back("b");
  keys.push_back("df");
  keys.push_back("gege");
  keys.push_back("hjhjhj");
  std::vector<std::string> values;
  std::vector<Status> ss = db_->MultiGet(ro, keys, &values);
  ASSERT_GE(ss.size(), 0);
  ASSERT_OK(ss[0]);
  ASSERT_NOK(ss[2]);
  std::vector<ColumnFamilyHandle*> cfs(2, db_->DefaultColumnFamily());
  std::vector<PinnableSlice> values2(keys.size());
  db_->MultiGet(ro, 2, cfs.data(), keys.data(), values2.data(), ss.data(),
                false);
  ASSERT_OK(ss[0]);
  db_->MultiGet(ro, db_->DefaultColumnFamily(), 2, keys.data() + 3,
                values2.data(), ss.data(), false);
  ASSERT_OK(db_->Get(ro, "a", &value));

  single_iter = db_->NewIterator(ro);
  single_iter->Seek("a");
  ASSERT_OK(single_iter->status());
  single_iter->SeekForPrev("b");
  ASSERT_OK(single_iter->status());
  single_iter->Next();
  ASSERT_OK(single_iter->status());
  delete single_iter;
  std::this_thread::sleep_for(std::chrono::seconds(1));

  db_->Get(ro, "g", &value).PermitUncheckedError();
  delete db_;
  ASSERT_OK(DestroyDB(dbname_, options));
}

}  // namespace ROCKSDB_NAMESPACE

int main(int argc, char** argv) {
  ROCKSDB_NAMESPACE::port::InstallStackTraceHandler();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
#endif  // GFLAG
#else
#include <stdio.h>

int main(int /*argc*/, char** /*argv*/) {
  fprintf(stderr, "Trace_analyzer test is not supported in ROCKSDB_LITE\n");
  return 0;
}

#endif  // !ROCKSDB_LITE  return RUN_ALL_TESTS();