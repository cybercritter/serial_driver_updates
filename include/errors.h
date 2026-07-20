/**
 * @file errors.h
 * @brief Error code definitions for the XR17V358 serial driver.
 *
 * This header defines a comprehensive set of error codes used throughout the
 * serial driver to communicate the results of operations. Callers should
 * check return values and respond appropriately to each error type.
 *
 * @author Serial Driver Development Team
 * @date 2026-07-20
 */

#ifndef ERRORS_H
#define ERRORS_H

/**
 * @defgroup ErrorCodes Error Code Enumeration
 * @brief All possible error codes returned by driver functions.
 * @{
 */

/**
 * @enum Error
 * @brief Enumeration of all possible error codes.
 *
 * Error codes are organized into logical groups for easier categorization
 * and handling. Error code values are non-zero except for ERROR_SUCCESS.
 */
typedef enum Error {
  /* Success */
  /**
   * @brief Operation completed successfully.
   * @details No error occurred; the operation returned the expected result.
   */
  ERROR_SUCCESS = 0,

  /* Parameter validation errors */
  /**
   * @brief Generic invalid parameter error.
   * @details A parameter was invalid but a more specific error doesn't apply.
   */
  ERROR_INVALID_PARAM,

  /**
   * @brief Invalid port number specified.
   * @details The port number is outside the valid range [0, MAX_NUM_PORTS].
   */
  ERROR_INVALID_PORT,

  /**
   * @brief Port number exceeds maximum value.
   * @details The requested port exceeds the device's port capacity.
   */
  ERROR_PORT_OUT_OF_RANGE,

  /* Pointer validation errors */
  /**
   * @brief Null pointer was provided when a valid pointer was required.
   * @details An output parameter was specified as NULL.
   */
  ERROR_NULL_PTR,

  /**
   * @brief Register base address is NULL or invalid.
   * @details The register memory mapping has not been initialized or is NULL.
   */
  ERROR_NULL_REGISTER_BASE,

  /* State errors */
  /**
   * @brief Device is in an invalid state for the requested operation.
   * @details The operation cannot be performed in the current device state.
   */
  ERROR_INVALID_STATE,

  /**
   * @brief Device has not been properly initialized.
   * @details Required initialization steps were not completed before use.
   */
  ERROR_DEVICE_NOT_INITIALIZED,

  /* Resource errors */
  /**
   * @brief Required resource is unavailable.
   * @details A required resource is busy or not available.
   */
  ERROR_RESOURCE_UNAVAILABLE,

  /**
   * @brief Operation timed out.
   * @details An operation did not complete within the expected time period.
   */
  ERROR_TIMEOUT,

  /* Generic errors */
  /**
   * @brief Generic operation failure.
   * @details An operation failed for an unspecified reason.
   */
  ERROR_FAILURE,

  /**
   * @brief Unknown or unclassified error.
   * @details An error occurred that doesn't fit into other categories.
   */
  ERROR_UNKNOWN
} Error;

/** @} */

#endif /* ERRORS_H */
