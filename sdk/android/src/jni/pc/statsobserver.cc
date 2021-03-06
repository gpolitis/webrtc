/*
 *  Copyright 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "sdk/android/src/jni/pc/statsobserver.h"

#include <vector>

#include "sdk/android/generated_peerconnection_jni/jni/StatsObserver_jni.h"
#include "sdk/android/generated_peerconnection_jni/jni/StatsReport_jni.h"
#include "sdk/android/src/jni/jni_helpers.h"

namespace webrtc {
namespace jni {

namespace {

jobject NativeToJavaStatsReportValue(
    JNIEnv* env,
    const rtc::scoped_refptr<StatsReport::Value>& value_ptr) {
  // Should we use the '.name' enum value here instead of converting the
  // name to a string?
  jstring j_name = NativeToJavaString(env, value_ptr->display_name());
  jstring j_value = NativeToJavaString(env, value_ptr->ToString());
  jobject ret = Java_Value_Constructor(env, j_name, j_value);
  env->DeleteLocalRef(j_name);
  env->DeleteLocalRef(j_value);
  return ret;
}

jobjectArray NativeToJavaStatsReportValueArray(
    JNIEnv* env,
    const StatsReport::Values& value_map) {
  // Ignore the keys and make an array out of the values.
  std::vector<StatsReport::ValuePtr> values;
  for (const auto& it : value_map)
    values.push_back(it.second);
  return NativeToJavaObjectArray(env, values,
                                 org_webrtc_StatsReport_00024Value_clazz(env),
                                 &NativeToJavaStatsReportValue);
}

jobject NativeToJavaStatsReport(JNIEnv* env, const StatsReport& report) {
  jstring j_id = NativeToJavaString(env, report.id()->ToString());
  jstring j_type = NativeToJavaString(env, report.TypeToString());
  jobjectArray j_values =
      NativeToJavaStatsReportValueArray(env, report.values());
  jobject ret = Java_StatsReport_Constructor(env, j_id, j_type,
                                             report.timestamp(), j_values);
  env->DeleteLocalRef(j_values);
  env->DeleteLocalRef(j_type);
  env->DeleteLocalRef(j_id);
  return ret;
}

}  // namespace

StatsObserverJni::StatsObserverJni(JNIEnv* jni, jobject j_observer)
    : j_observer_global_(jni, j_observer) {}

void StatsObserverJni::OnComplete(const StatsReports& reports) {
  JNIEnv* env = AttachCurrentThreadIfNeeded();
  jobjectArray j_reports =
      NativeToJavaObjectArray(env, reports, org_webrtc_StatsReport_clazz(env),
                              [](JNIEnv* env, const StatsReport* report) {
                                return NativeToJavaStatsReport(env, *report);
                              });
  Java_StatsObserver_onComplete(env, *j_observer_global_, j_reports);
  env->DeleteLocalRef(j_reports);
}

}  // namespace jni
}  // namespace webrtc
