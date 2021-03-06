<?php

/**
 * @file
 * Tourney Series admin forms/pages.
 */

/**
 * The tournament series form.
 */
function tourney_series_form($form, &$form_state, $series) {
  module_load_include('inc', 'tourney', 'includes/tourney.tournament');

  // Set the bundle name
  $series->type = 'series';
  $form_state['tourney'] = $series;

  $form['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Series Name'),
    '#description' => t('Enter the name of this series.'),
    '#size' => 40,
    '#maxlength' => 255,
    '#default_value' => isset($series->label) ? $series->label : '',
    '#weight' => -10,
  );

  $form['name'] = array(
    '#type' => 'machine_name',
    '#default_value' => isset($series->label) ? $series->name : '',
    '#description' => t('A unique name for tournament series. It must only contain lowercase letters, numbers and hyphens.'),
    '#machine_name' => array(
      'source' => array('label'),
      'exists' => 'tourney_series_name_exists',
      'replace_pattern' => '[^a-z0-9-]+',
      'replace' => '-',
    ),
    '#weight' => -9,
  );

  $form['format'] = array(
    '#type' => 'select',
    '#title' => t('Series Logic'),
    '#description' => t('Choose the logic defined for this series.'),
    '#options' => tourney_get_type_options(false, tourney_series_get_types()),
    '#default_value' => isset($series->format) ? $series->format : -1,
    '#weight' => -8,
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 10,
  );

  if (!empty($series->id)) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#weight' => 40,
      '#submit' => array('tourney_series_delete_submit'),
    );
  }

  $form['additional_settings'] = array(
    '#type' => 'vertical_tabs',
    '#weight' => 99,
  );

  $form['options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Publishing Options'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'additional_settings',
    '#weight' => 95,
  );

  $form['options']['status'] = array(
    '#type' => 'checkbox',
    '#title' => t('Published'),
    '#default_value' => isset($series->status) ? $series->status : 0,
  );

  field_attach_form('tourney_series', $series, $form, $form_state);

  $form['#submit'][] = 'tourney_series_form_submit';
  return $form;
}

/**
 * Tournament form submit handler.
 */
function tourney_series_form_submit($form, &$form_state) {
  $series = $form_state['tourney'];
  entity_form_submit_build_entity('tourney_series', $series, $form, $form_state);
  tourney_series_save($series);

  drupal_set_message(t('Your tournament series has been saved.'));
  $form_state['redirect'] = $series->getUri();
}

/**
 * Wrap the same submit handler in tourney_tournament.
 */
function tourney_series_delete_submit($form, &$form_state) {
  module_load_include('inc', 'tourney', 'tourney.tournament');
  return tourney_tournament_delete_submit($form, $form_state);
}

/**
 * Page callback. Edit a series.
 */
function tourney_series_edit($series) {
  drupal_set_title(t('Edit Series: @title', array('@title' => $series->label)));

  return drupal_get_form('tourney_series_form', $series);
}

/**
 * Machine name callback to make sure it name does not already exist.
 */
function tourney_series_name_exists($value) {
  return db_query_range('SELECT 1 FROM {tourney_series} WHERE name = :name', 0, 1, array(':name' => $value))->fetchField();
}

/**
 * Show the series page.
 */
function tourney_series_page_view($series, $view_mode = 'full') {
  $entity_info = entity_get_info('tourney_series');
  $mode = array_key_exists($view_mode, $entity_info['view modes'])
    ? $view_mode : 'full';
  return $series->view($mode, NULL, TRUE);
}

function tourney_series_confirm_delete_page($series) {
  if ($series) {
    return drupal_get_form('tourney_series_confirm_delete', $series);
  }
  return MENU_NOT_FOUND;
}

function tourney_series_confirm_delete($form, &$form_state, $series) {
  $form['#series'] = $series;
  $form['#tid'] = $series->id;
  return confirm_form(
    $form,
    t('Are you sure you want to delete the series %title?', array('%title' => $series->label)),
    TourneyMatchEntity::path($series->id, 'tourney_series'),
    t('Tournaments will not be deleted, they will become orphaned. This action cannot be undone.'),
    t('Delete'),
    t('Cancel'),
    'tourney_series_confirm_delete');
}

/**
 * Submit handler after confirming deletion.
 */
function tourney_series_confirm_delete_submit($form, &$form_state) {
  // Delete the series.
  $form['#series']->delete();
  $form_state['redirect'] = "admin/content/tourney";
}

/**
 * Generates the tourney type editing form.
 */
function tourney_series_type_form($form, &$form_state, $tourney_series_type, $op = 'edit') {
  if ($op == 'clone') {
    $tourney_series_type->label .= ' (cloned)';
    $tourney_series_type->type = '';
  }

  $form['label'] = array(
    '#title' => t('Label'),
    '#type' => 'textfield',
    '#default_value' => isset($tourney_series_type->label) ? $tourney_series_type->label : '',
    '#description' => t('The human-readable name of this type.'),
    '#required' => TRUE,
    '#size' => 30,
  );
  // Machine-readable type name.
  $form['type'] = array(
    '#type' => 'machine_name',
    '#default_value' => isset($tourney_series_type->type) ? $tourney_series_type->type : '',
    '#maxlength' => 32,
    '#machine_name' => array(
      'exists' => $form_state['entity_type'] . '_get_types',
      'source' => array('label'),
    ),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save tournament series type'),
    '#weight' => 40,
  );

  if ($op != 'add' && $op != 'clone') {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete tournament series type'),
      '#weight' => 45,
      '#limit_validation_errors' => array(),
      '#submit' => array('tourney_series_tournament_type_form_submit_delete')
    );
  }

  return $form;
}

/**
 * Form API submit callback for the type form.
 */
function tourney_series_tournament_type_form_submit(&$form, &$form_state) {
  $tourney_series_tournament_type = entity_ui_form_submit_build_entity($form, $form_state);
  // Save and go back.
  $tourney_series_tournament_type->save();
  $form_state['redirect'] = 'admin/structure/tournament';
}

/**
 * Form API submit callback for the delete button.
 */
function tourney_series_tournament_type_form_submit_delete(&$form, &$form_state) {
  drupal_set_message(t('The tournament and all its matches and games have been deleted.'));
  $form_state['redirect'] = 'admin/structure/tournament/manage/' . $form_state['tourney_series_tournament_type']->type . '/delete';
}