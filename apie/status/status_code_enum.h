#pragma once

namespace apie {
namespace status {


enum class StatusCode {
  /// Not an error; returned on success.
  OK = 0,
  OK_ASYNC = 1,

  /// The operation was cancelled (typically by the caller).
  CANCELLED = 2,

  /// Unknown error. An example of where this error may be returned is if a
  /// Status value received from another address space belongs to an error-space
  /// that is not known in this address space. Also errors raised by APIs that
  /// do not return enough error information may be converted to this error.
  UNKNOWN = 3,

  /// Client specified an invalid argument. Note that this differs from
  /// FAILED_PRECONDITION. INVALID_ARGUMENT indicates arguments that are
  /// problematic regardless of the state of the system (e.g., a malformed file
  /// name).
INVALID_ARGUMENT = 4,

/// Deadline expired before operation could complete. For operations that
/// change the state of the system, this error may be returned even if the
/// operation has completed successfully. For example, a successful response
/// from a server could have been delayed long enough for the deadline to
/// expire.
DEADLINE_EXCEEDED = 5,

/// Some requested entity (e.g., file or directory) was not found.
NOT_FOUND = 6,

/// Some entity that we attempted to create (e.g., file or directory) already
/// exists.
ALREADY_EXISTS = 7,

/// The caller does not have permission to execute the specified operation.
/// PERMISSION_DENIED must not be used for rejections caused by exhausting
/// some resource (use RESOURCE_EXHAUSTED instead for those errors).
/// PERMISSION_DENIED must not be used if the caller can not be identified
/// (use UNAUTHENTICATED instead for those errors).
PERMISSION_DENIED = 8,

/// The request does not have valid authentication credentials for the
/// operation.
UNAUTHENTICATED = 9,

/// Some resource has been exhausted, perhaps a per-user quota, or perhaps the
/// entire file system is out of space.
RESOURCE_EXHAUSTED = 10,

/// Operation was rejected because the system is not in a state required for
/// the operation's execution. For example, directory to be deleted may be
/// non-empty, an rmdir operation is applied to a non-directory, etc.
///
/// A litmus test that may help a service implementor in deciding
/// between FAILED_PRECONDITION, ABORTED, and UNAVAILABLE:
///  (a) Use UNAVAILABLE if the client can retry just the failing call.
///  (b) Use ABORTED if the client should retry at a higher-level
///      (e.g., restarting a read-modify-write sequence).
///  (c) Use FAILED_PRECONDITION if the client should not retry until
///      the system state has been explicitly fixed. E.g., if an "rmdir"
///      fails because the directory is non-empty, FAILED_PRECONDITION
///      should be returned since the client should not retry unless
///      they have first fixed up the directory by deleting files from it.
///  (d) Use FAILED_PRECONDITION if the client performs conditional
///      REST Get/Update/Delete on a resource and the resource on the
///      server does not match the condition. E.g., conflicting
///      read-modify-write on the same resource.
FAILED_PRECONDITION = 11,

/// The operation was aborted, typically due to a concurrency issue like
/// sequencer check failures, transaction aborts, etc.
///
/// See litmus test above for deciding between FAILED_PRECONDITION, ABORTED,
/// and UNAVAILABLE.
ABORTED = 12,

/// Operation was attempted past the valid range. E.g., seeking or reading
/// past end of file.
///
/// Unlike INVALID_ARGUMENT, this error indicates a problem that may be fixed
/// if the system state changes. For example, a 32-bit file system will
/// generate INVALID_ARGUMENT if asked to read at an offset that is not in the
/// range [0,2^32-1], but it will generate OUT_OF_RANGE if asked to read from
/// an offset past the current file size.
///
/// There is a fair bit of overlap between FAILED_PRECONDITION and
/// OUT_OF_RANGE. We recommend using OUT_OF_RANGE (the more specific error)
/// when it applies so that callers who are iterating through a space can
/// easily look for an OUT_OF_RANGE error to detect when they are done.
OUT_OF_RANGE = 13,

/// Operation is not implemented or not supported/enabled in this service.
UNIMPLEMENTED = 14,

/// Internal errors. Means some invariants expected by underlying System has
/// been broken. If you see one of these errors, Something is very broken.
INTERNAL = 15,

/// The service is currently unavailable. This is a most likely a transient
/// condition and may be corrected by retrying with a backoff. Note that it is
/// not always safe to retry non-idempotent operations.
///
/// \warning Although data MIGHT not have been transmitted when this
/// status occurs, there is NOT A GUARANTEE that the server has not seen
/// anything. So in general it is unsafe to retry on this status code
/// if the call is non-idempotent.
///
/// See litmus test above for deciding between FAILED_PRECONDITION, ABORTED,
/// and UNAVAILABLE.
UNAVAILABLE = 16,

/// Unrecoverable data loss or corruption.
DATA_LOSS = 17,
TIMEOUT = 18,
HOOK_ERROR = 19,

LoadFromDbError = 20,
NotMatchedResultError = 21,
DirtyFlagZero = 22,
DB_InsertError = 23,
DB_LoadedError = 24,
DB_BindTableError = 25,
DB_MysqlQueryError = 26,

Obj_NotExist = 50,

RPC_Request_UnRegister = 60,
RPC_Request_createMessage = 61,
RPC_Request_ParseFromString = 62,

/// Force users to include a default branch:
  DO_NOT_USE = -1
};


}
}
