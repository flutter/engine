// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_METHOD_RESPONSE_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_METHOD_RESPONSE_H_

#if !defined(__FLUTTER_LINUX_INSIDE__) && !defined(FLUTTER_LINUX_COMPILATION)
#error "Only <flutter_linux/flutter_linux.h> can be included directly."
#endif

#include <glib-object.h>

#include "fl_value.h"

G_BEGIN_DECLS

/**
 * FlMethodResponseError:
 * @FL_METHOD_RESPONSE_ERROR_FAILED: Call failed due to an unspecified error.
 * @FL_METHOD_RESPONSE_ERROR_REMOTE_ERROR: An error was returned by the other
 * side of the channel.
 * @FL_METHOD_RESPONSE_ERROR_NOT_IMPLEMENTED: The requested method is not
 * implemented.
 *
 * Errors for #FlMessageResponse objects to set on failures.
 */
#define FL_METHOD_RESPONSE_ERROR fl_method_response_error_quark()

typedef enum {
  FL_METHOD_RESPONSE_ERROR_FAILED,
  FL_METHOD_RESPONSE_ERROR_REMOTE_ERROR,
  FL_METHOD_RESPONSE_ERROR_NOT_IMPLEMENTED,
} FlMethodResponseError;

GQuark fl_method_response_error_quark(void) G_GNUC_CONST;

G_DECLARE_DERIVABLE_TYPE(FlMethodResponse,
                         fl_method_response,
                         FL,
                         METHOD_RESPONSE,
                         GObject)

struct _FlMethodResponseClass {
  GObjectClass parent_class;
};

G_DECLARE_FINAL_TYPE(FlMethodSuccessResponse,
                     fl_method_success_response,
                     FL,
                     METHOD_SUCCESS_RESPONSE,
                     FlMethodResponse)

G_DECLARE_FINAL_TYPE(FlMethodErrorResponse,
                     fl_method_error_response,
                     FL,
                     METHOD_ERROR_RESPONSE,
                     FlMethodResponse)

G_DECLARE_FINAL_TYPE(FlMethodNotImplementedResponse,
                     fl_method_not_implemented_response,
                     FL,
                     METHOD_NOT_IMPLEMENTED_RESPONSE,
                     FlMethodResponse)

/**
 * FlMethodResponse:
 *
 * #FlMethodResponse contains the information returned when a #FlMethodChannel
 * method call returns successfully.
 */

/**
 * FlMethodSuccessResponse:
 *
 * #FlMethodError contains the information returned when a #FlMethodChannel
 * method call returns successfully.
 */

/**
 * FlMethodErrorResponse:
 *
 * #FlMethodError contains the information returned when a #FlMethodChannel
 * method call returns an error.
 */

/**
 * FlMethodNotImplementedResponse:
 *
 * #FlMethodError indicates when a #FlMethodChannel method call is not
 * implemented on the other side of the channel.
 */

/**
 * fl_method_response_get_result:
 * @response: a #FlMethodResponse
 * @error: (allow-none): #GError location to store the error occurring, or %NULL
 * to ignore.
 *
 * Gets the result of a method call, or an error if the response wasn't
 * successful.
 *
 * Returns: a #FlValue or %NULL on error.
 */
FlValue* fl_method_response_get_result(FlMethodResponse* response,
                                       GError** error);

/**
 * fl_method_success_response_new:
 * @result: the #FlValue returned by the method call
 *
 * Creates a response to a method call when that method has successfully
 * completed.
 *
 * Returns: a new #FlMethodResponse.
 */
FlMethodSuccessResponse* fl_method_success_response_new(FlValue* result);

/**
 * fl_method_success_response_get_result:
 * @response: an #FlMethodSuccessResponse
 *
 * Gets the result of the method call.
 *
 * Returns: an #FlValue
 */
FlValue* fl_method_success_response_get_result(
    FlMethodSuccessResponse* response);

/**
 * fl_method_error_response_new:
 * @result: a #FlValue
 * @code: an error code
 * @message: (allow-none): an error message.
 * @details: (allow-none): error details.
 *
 * Creates a response to a method call when that method has returned an error.
 *
 * Returns: a new #FlMethodErrorResponse.
 */
FlMethodErrorResponse* fl_method_error_response_new(const gchar* code,
                                                    const gchar* message,
                                                    FlValue* details);

/**
 * fl_method_error_response_get_code:
 * @response: an #FlMethodErrorResponse
 *
 * Gets the error code reported.
 *
 * Returns: an error code.
 */
const gchar* fl_method_error_response_get_code(FlMethodErrorResponse* response);

/**
 * fl_method_error_response_get_message:
 * @response: an #FlMethodErrorResponse
 *
 * Gets the error message reported.
 *
 * Returns: an error message or %NULL if no error message provided.
 */
const gchar* fl_method_error_response_get_message(
    FlMethodErrorResponse* response);

/**
 * fl_method_error_response_get_details:
 * @response: an #FlMethodErrorResponse
 *
 * Gets the details provided with this error.
 *
 * Returns: an #FlValue or %NULL if no details provided.
 */
FlValue* fl_method_error_response_get_details(FlMethodErrorResponse* response);

/**
 * fl_method_not_implemented_response_new:
 *
 * Creates a response to a method call when that method does not exist.
 *
 * Returns: a new #FlMethodNotImplementedResponse.
 */
FlMethodNotImplementedResponse* fl_method_not_implemented_response_new();

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_METHOD_RESPONSE_H_
