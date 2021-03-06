// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_BASE_MEDIA_LOG_H_
#define MEDIA_BASE_MEDIA_LOG_H_

#include <string>

#include <memory>
#include <sstream>

#include <utility>

#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
//#include "base/thread_annotations.h"
#include "base/buffering_state.h"
#include "base/media_export.h"
#include "base/media_log_event.h"
//#include "media/base/media_log_properties.h"
//#include "base/pipeline_impl.h"
#include "base/pipeline_status.h"
//#include "url/gurl.h"

namespace media {

struct PipelineImpl
{
    enum State {
        kCreated,
        kStarting,
        kSeeking,
        kPlaying,
        kStopping,
        kStopped,
        kSuspending,
        kSuspended,
        kResuming,
    };
};

// Interface for media components to log to chrome://media-internals log.
//
// To provide a logging implementation, derive from MediaLog instead.
//
// Implementations only need to implement AddEventLocked(), which must be thread
// safe in the sense that it may be called from multiple threads, though it will
// not be called concurrently.  See below for more details.
//
// Implementations should also call InvalidateLog during destruction, to signal
// to any child logs that the underlying log is no longer available.
class MediaLog {
 public:
  enum MediaLogLevel {
    // Fatal error, e.g. cause of playback failure. Since this is also used to
    // form MediaError.message, do NOT use this for non-fatal errors to avoid
    // contaminating MediaError.message.
    MEDIALOG_ERROR,

    // Warning about non-fatal issues, e.g. quality of playback issues such as
    // audio/video out of sync.
    MEDIALOG_WARNING,

    // General info useful for Chromium and/or web developers, testers and even
    // users, e.g. audio/video codecs used in a playback instance.
    MEDIALOG_INFO,

    // Misc debug info for Chromium developers.
    MEDIALOG_DEBUG,
  };

  // Convert various enums to strings.
  static std::string MediaLogLevelToString(MediaLogLevel level);
  static MediaLogEvent::Type MediaLogLevelToEventType(MediaLogLevel level);
  static std::string EventTypeToString(MediaLogEvent::Type type);

  static std::string BufferingStateToString(
      BufferingState state,
      BufferingStateChangeReason reason = BUFFERING_CHANGE_REASON_UNKNOWN);

  static std::string MediaEventToLogString(const MediaLogEvent& event);

  // Returns a string usable as part of a MediaError.message, for only
  // PIPELINE_ERROR or MEDIA_ERROR_LOG_ENTRY events, with any newlines replaced
  // with whitespace in the latter kind of events.
  static std::string MediaEventToMessageString(const MediaLogEvent& event);

  // Constructor is protected, see below.

  virtual ~MediaLog();

  // Add an event to this log.  Inheritors should override AddEventLocked to
  // do something.
  void AddEvent(std::unique_ptr<MediaLogEvent> event);

  // Notify the media log that the player is destroyed. Some implementations
  // will want to change event handling based on this.
  void OnWebMediaPlayerDestroyed();

  // Returns a string usable as the contents of a MediaError.message.
  // This method returns an incomplete message if it is called before the
  // pertinent events for the error have been added to the log.
  // Note: The base class definition only produces empty messages. See
  // RenderMediaLog for where this method is meaningful.
  // Inheritors should override GetErrorMessageLocked().
  std::string GetErrorMessage();

  // Helper methods to create events and their parameters.
  std::unique_ptr<MediaLogEvent> CreateEvent(MediaLogEvent::Type type);
  std::unique_ptr<MediaLogEvent> CreateBooleanEvent(MediaLogEvent::Type type,
                                                    const std::string& property,
                                                    bool value);
  std::unique_ptr<MediaLogEvent> CreateCreatedEvent(
      const std::string& origin_url);
  std::unique_ptr<MediaLogEvent> CreateStringEvent(MediaLogEvent::Type type,
                                                   const std::string& property,
                                                   const std::string& value);
  std::unique_ptr<MediaLogEvent> CreateTimeEvent(MediaLogEvent::Type type,
                                                 const std::string& property,
                                                 base::TimeDelta value);
  std::unique_ptr<MediaLogEvent> CreateTimeEvent(MediaLogEvent::Type type,
                                                 const std::string& property,
                                                 double value);
  std::unique_ptr<MediaLogEvent> CreateLoadEvent(const std::string& url);
  std::unique_ptr<MediaLogEvent> CreatePipelineStateChangedEvent(
      PipelineImpl::State state);
  std::unique_ptr<MediaLogEvent> CreatePipelineErrorEvent(PipelineStatus error);
  std::unique_ptr<MediaLogEvent> CreateVideoSizeSetEvent(size_t width,
                                                         size_t height);
  std::unique_ptr<MediaLogEvent> CreateBufferingStateChangedEvent(
      const std::string& property,
      BufferingState state,
      BufferingStateChangeReason reason);

  // Report a log message at the specified log level.
  void AddLogEvent(MediaLogLevel level, const std::string& message);

  // Only way to access the MediaLogEvent::PROPERTY_CHANGE event type,
  // so that parameter types can be checked from media_log_properties.h.
//  template <MediaLogProperty P, typename T>
//  void SetProperty(const T& value) {
//    AddEvent(CreatePropertyEvent<P, T>(value));
//  }

  // Getter for |id_|. Used by MojoMediaLogService to construct MediaLogEvents
  // to log into this MediaLog. Also used in trace events to associate each
  // event with a specific media playback.
  int32_t id() const { return id_; }

  // Provide a MediaLog which can have a separate lifetime from this one, but
  // still write to the same log.  It is not guaranteed that this will log
  // forever; it might start silently discarding log messages if the original
  // log is closed by whoever owns it.
  virtual std::unique_ptr<MediaLog> Clone();

 protected:
  // Ensures only subclasses and factories (e.g. Clone()) can create MediaLog.
  MediaLog();

  // Methods that may be overridden by inheritors.  All calls may arrive on any
  // thread, but will be synchronized with respect to any other *Locked calls on
  // any other thread, and with any parent log invalidation.
  //
  // Please see the documentation for the corresponding public methods.
  virtual void AddEventLocked(std::unique_ptr<MediaLogEvent> event);
  virtual void OnWebMediaPlayerDestroyedLocked();
  virtual std::string GetErrorMessageLocked();

  // MockMediaLog also needs to call this method.
//  template <MediaLogProperty P, typename T>
//  std::unique_ptr<MediaLogEvent> CreatePropertyEvent(const T& value) {
//    auto event = CreateEvent(MediaLogEvent::PROPERTY_CHANGE);
//    event->params.SetKey(MediaLogPropertyKeyToString(P),
//                         MediaLogPropertyTypeSupport<P, T>::Convert(value));
//    return event;
//  }

  // Notify all child logs that they should stop working.  This should be called
  // to guarantee that no further calls into AddEvent should be allowed.
  // Further, since calls into this log may happen on any thread, it's important
  // to call this while the log is still in working order.  For example, calling
  // it immediately during destruction is a good idea.
  void InvalidateLog();

  struct ParentLogRecord : base::RefCountedThreadSafe<ParentLogRecord> {
    ParentLogRecord(MediaLog* log);

    // |lock_| protects the rest of this structure.
    //base::Lock lock;

    // Original media log, or null.
    MediaLog* media_log = nullptr;

   protected:
    friend class base::RefCountedThreadSafe<ParentLogRecord>;
    virtual ~ParentLogRecord();

    DISALLOW_COPY_AND_ASSIGN(ParentLogRecord);
  };

  // Use |parent_log_record| instead of making a new one.
  MediaLog(scoped_refptr<ParentLogRecord> parent_log_record);

 private:
  // Allows MediaLogTest to construct MediaLog directly for testing.
  friend class MediaLogTest;
//  FRIEND_TEST_ALL_PREFIXES(MediaLogTest, EventsAreForwarded);
//  FRIEND_TEST_ALL_PREFIXES(MediaLogTest, EventsAreNotForwardedAfterInvalidate);

  enum : size_t {
    // Max length of URLs in Created/Load events. Exceeding triggers truncation.
    kMaxUrlLength = 1000,
  };

  // URLs (for Created and Load events) may be of arbitrary length from the
  // untrusted renderer. This method truncates to |kMaxUrlLength| before storing
  // the event, and sets the last 3 characters to an ellipsis.
  static std::string TruncateUrlString(std::string log_string);

  // The underlying media log.
  scoped_refptr<ParentLogRecord> parent_log_record_;

  // A unique (to this process) id for this MediaLog.
  int32_t id_;
  DISALLOW_COPY_AND_ASSIGN(MediaLog);
};

// Helper class to make it easier to use MediaLog like DVLOG().
class LogHelper {
 public:
  LogHelper(MediaLog::MediaLogLevel level, MediaLog* media_log);
  LogHelper(MediaLog::MediaLogLevel level,
            const std::unique_ptr<MediaLog>& media_log);
  ~LogHelper();

  std::ostream& stream() { return stream_; }

 private:
  const MediaLog::MediaLogLevel level_;
  MediaLog* const media_log_;
  std::stringstream stream_;
};

// Provides a stringstream to collect a log entry to pass to the provided
// MediaLog at the requested level.
#define MEDIA_LOG(level, media_log) \
  LogHelper((MediaLog::MEDIALOG_##level), (media_log)).stream()

// Logs only while |count| < |max|, increments |count| for each log, and warns
// in the log if |count| has just reached |max|.
// Multiple short-circuit evaluations are involved in this macro:
// 1) LAZY_STREAM avoids wasteful MEDIA_LOG and evaluation of subsequent stream
//    arguments if |count| is >= |max|, and
// 2) the |condition| given to LAZY_STREAM itself short-circuits to prevent
//    incrementing |count| beyond |max|.
// Note that LAZY_STREAM guarantees exactly one evaluation of |condition|, so
// |count| will be incremented at most once each time this macro runs.
// The "|| true" portion of |condition| lets logging occur correctly when
// |count| < |max| and |count|++ is 0.
// TODO(wolenetz,chcunningham): Consider using a helper class instead of a macro
// to improve readability.
#define LIMITED_MEDIA_LOG(level, media_log, count, max)                       \
  LAZY_STREAM(MEDIA_LOG(level, media_log),                                    \
              (count) < (max) && ((count)++ || true))                         \
      << (((count) == (max)) ? "(Log limit reached. Further similar entries " \
                               "may be suppressed): "                         \
                             : "")

}  // namespace media

#endif  // MEDIA_BASE_MEDIA_LOG_H_
