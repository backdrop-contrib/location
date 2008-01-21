
/**
 * This jQuery function will force the Province field
 * to autcomplete the pronvice for a given country
 */
$(function() {
  // Record the original URL
  var pronvice = $("#edit-province-autocomplete").val() + '/';

  // Set the country code at the begging
  if ($("#edit-country").val()) {
    $("#edit-province-autocomplete").val(pronvice + $("#edit-country").val());
  }

  // Change the country code everytime the country field changes
  $("#edit-country").change(function() {
    $("#edit-province-autocomplete").val(pronvice + $(this).val());
    Drupal.autocompleteAutoAttach();
  })
});
