<?php
// $Id$

/**
 * Admin settings form.
 *
 * @ingroup form
 */
function _location_admin_settings() {
  cache_clear_all('location:supported-countries', 'cache');
  $supported_countries = _location_supported_countries();

  $default_country = variable_get('location_default_country', 'us');

  $configured_countries = location_get_configured_countries();

  $iso_list_sorted = location_get_iso3166_list();
  array_multisort($iso_list_sorted);
  $iso_list_sorted = array_merge(array('' => ''), $iso_list_sorted);

  if ($default_country && in_array($default_country, array_keys($supported_countries)) && !in_array($default_country, $configured_countries)) {
    $configured_countries = variable_get('location_configured_countries', array('us' => 1));
    $configured_countries[$default_country] = 1;
    variable_set('location_configured_countries', $configured_countries);
    // clear the views cache to pick up any changes
    if (function_exists('views_invalidate_cache')) {
      views_invalidate_cache();
    }
  }

  $form  = array();
  $form['location_default_country'] = array(
    '#type'           => 'select',
    '#title'          => t('Default country selection'),
    '#default_value'  => variable_get('location_default_country', 'us'),
    '#options'        => $iso_list_sorted,
    '#description'    => t('This will be the country that is automatically selected when a location form is served for a new location.')
  );
  $form['location_suppress_country'] = array(
    '#type'           => 'checkbox',
    '#title'          => t('Hide country selection'),
    '#return_value'   => 1,
    '#default_value'  => variable_get('location_suppress_country', 0),
    '#description'    => t("If your site is specific to a country and you would like to hide the country field on search forms and content creation forms, check this box.  Doing so will automatically assume the country to be the country you have chosen for the 'default country selection' above.")
  );
  $form['location_search_distance_unit'] = array(
    '#type'           => 'radios',
    '#title'          => t('Distance unit for location-based searches'),
    '#default_value'  => variable_get('location_search_distance_unit', 0),
    '#options'        => array(
      0       => t('Allow both miles and kilometers.'),
      'mile'  => t('Only allow a search-radius to be specified in miles'),
      'km'    => t('Only allow a search-radius to be specified in kilometers')
    ),
    '#description'    => t('Select the distance unit that applies when users search for content by a specified location and search-radius.')
  );
  $form['location_display_location'] = array(
    '#type'           => 'radios',
    '#title'          => t('Toggle location display'),
    '#default_value'  => variable_get('location_display_location', 1),
    '#options'        => array(
      0 => t('Disable the display of locations.'),
      1 => t('Enable the display of locations.')
    ),
    '#description'    => t('If you are interested in turning off locations and having a custom theme control their display, you may want to disable the display of locations so your theme can take that function.')
  );

  $form['location_usegmap'] = array(
    '#type'           => 'checkbox',
    '#title'          => t('Use a Google Map to set latitude and longitude '),
    '#default_value'  => variable_get('location_usegmap', FALSE),
    '#description'    => t('If the gmap.module is installed and <a href="@enabled">enabled</a>, and this is setting is turned on, users that are allowed to manually enter latitude/longitude coordinates will be able to do so with an interactive Google Map.  You should also make sure you have entered a <a href="@google_maps_api_key">Google Maps API key</a> into your <a href="@gmap_module_settings">gmap module settings</a>.', array('@enabled' => url('admin/build/modules'), '@google_maps_api_key' => 'http://www.google.com/apis/maps', '@gmap_module_settings' => url('admin/settings/gmap'))),
// @@@ megapatch This is an idea, but I'd opt more for a warning here...
//    '#disabled'       => !module_exists('gmap'),
  );

  $form['location_garbagecollect'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable garbage collection of unused locations'),
    '#default_value' => variable_get('location_garbagecollect', TRUE),
    '#description' => t('If checked, the system can automatically garbage collect locations that are no longer referenced anywhere. It is recommended that you leave this checked.'),
  );

  $form['countries'] = array(
    '#collapsible'    => TRUE,
    '#collapsed'      => TRUE,
    '#description'    => t('Currently, your Drupal site is capable of supporting extra features (e.g., postal code proximity searches) for locations from this list of countries. You can narrow the list down to countries for which you want to support these extra features. It may be useful for performance to narrow down this list if most the locations in your system are from only a handful of the listed countries.'),
    '#title'          => t('Countries'),
    '#tree'           => FALSE,
    '#type'           => 'fieldset',
  );

  $form['countries']['location_configured_countries'] = array(
    '#type'           => 'checkboxes',
    '#title'          => t('Enable all <em>available</em> features for locations from the following countries'),
    '#default_value'  => location_get_configured_countries(),
    '#options'        => $supported_countries,
  );

  return system_settings_form($form);
}

/**
 * Settings page for map links.
 */
function _location_map_link_options_form() {
  $form = array();

  $form['countries'] = array(
    '#type' => 'markup',
    '#value' => ''
  );

  foreach (location_configured_countries() as $country_iso => $country_name) {
    location_load_country($country_iso);

    $form['countries'][$country_iso] = array(
      '#type' => 'markup',
      '#value' => ''
    );

    $form['countries'][$country_iso]['label_'. $country_iso] = array(
      '#type' => 'markup',
      '#value' => $country_name
    );

    // Set up '#options' array for mapping providers for the current country
    $mapping_options = array();
    $provider_function = 'location_map_link_'. $country_iso .'_providers';
    $default_provider_function = 'location_map_link_'. $country_iso .'_default_providers';
    $checked = variable_get('location_map_link_'. $country_iso, function_exists($default_provider_function) ? $default_provider_function() : array());
    //print "Calling provider function $provider_function";
    if (function_exists($provider_function)) {
      foreach ($provider_function() as $name => $details) {
        $mapping_options[$name] = '<a href="'. $details['url'] .'">'. $details['name'] .'</a> (<a href="'. $details['tos'] .'">Terms of Use</a>)';
      }
    }

    if (count($mapping_options)) {
      $form['countries'][$country_iso]['location_map_link_'. $country_iso] = array(
        '#title' => '',
        '#type' => 'checkboxes',
        '#default_value' => $checked,
        '#options' => $mapping_options
      );
    }
    else {
      $form['countries'][$country_iso]['location_map_link_'. $country_iso] = array(
        '#type' => 'markup',
        '#value' => t('None supported.')
      );
    }
  }

  $form['#theme'] = 'location_map_link_options';
  return system_settings_form($form);
}
