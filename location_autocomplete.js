// $Id$

/**
 * Twiddle the province autocomplete whenever the user changes the country.
 */
Drupal.behaviors.location = function(context) {
  $('select.location_auto_country:not(.location-processed)', context).change(function(e) {
    //Unbind events on province field and empty its value
    var input = $('.location_auto_province', $(this).parents('fieldset:first')).unbind().val('');
    //Get the (hidden) *-autocomplete input element
    var input_autocomplete = $('#' + input.attr('id') + '-autocomplete');
    // Update autocomplete url
    input_autocomplete.val(input_autocomplete.val().substr(0, input_autocomplete.val().length - 2) + $(this).val());
    // Re-attach events
    input_autocomplete.removeClass('autocomplete-processed');
    Drupal.behaviors.autocomplete($(this).parents('fieldset:first'));
  }).addClass('location-processed');
};
