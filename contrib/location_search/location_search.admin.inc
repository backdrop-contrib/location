<?php
// $Id$

/**
 * @file
 * Administration related forms.
 */

/**
 * Admin settings form.
 *
 * @ingroup form
 */
function _location_search_admin_settings() {
  $form['location_suppress_country'] = array(
    '#type'           => 'checkbox',
    '#title'          => t('Hide country selection'),
    '#return_value'   => 1,
    '#default_value'  => variable_get('location_suppress_country', 0),
    '#description'    => t("If your site is specific to a country and you would like to hide the country field on search forms, check this box.  Doing so will automatically assume the country to be the country you have chosen for the 'default country selection' on the Main settings tab.."),
  );
  $form['location_search_distance_unit'] = array(
    '#type'           => 'radios',
    '#title'          => t('Distance unit for location-based searches'),
    '#default_value'  => variable_get('location_search_distance_unit', 0),
    '#options'        => array(
      0       => t('Allow both miles and kilometers.'),
      'mile'  => t('Only allow a search-radius to be specified in miles'),
      'km'    => t('Only allow a search-radius to be specified in kilometers'),
    ),
    '#description'    => t('Select the distance unit that applies when users search for content by a specified location and search-radius.'),
  );
  return system_settings_form($form);
}
