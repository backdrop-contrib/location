/* $Id$ */

/**
 * @file
 * Autocompletion for Province field.
 * This jQuery function will force the Province field
 * to autcomplete the pronvice for a given country.
 */

$(function() {
  // Use this ID to 'simplify' the callings
  var id = '#edit-locations-';

  // Record the original URL
  var url = $(id + '0-autocomplete').val() + '/';

  // Get all locations
  for (var n = 0; true; n++) {
    // Check if this Field exists.
    // If not, stop the script
    if (!$(id + n +'-country').id()) {
      break;
    }

    // Set the country code at the beginning
    if ($(id + n +'-country').val()) {
      $(id + n +'-autocomplete').val(url + $(id + n +'-country').val());
    }

    // Change the country code everytime the country field changes
    $(id + n +'-country').change(function() {
      $(id + n +'-autocomplete').val(url + $(this).val());
      Drupal.autocompleteAutoAttach();
    })
  }
});
