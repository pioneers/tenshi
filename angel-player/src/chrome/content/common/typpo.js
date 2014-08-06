/* jshint globalstrict: true */
"use strict";

const typpo_module = require('tenshi/common/factory');
const url = require('sdk/url');

const PIEMOS_FRAMING_YAML_FILE =
    'chrome://angel-player/content/common_defs/legacy_piemos_framing.yaml';

const XBEE_FRAMING_YAML_FILE =
    'chrome://angel-player/content/common_defs/xbee_typpo.yaml';

const CONFIG_YAML_FILE =
    'chrome://angel-player/content/common_defs/config.yaml';


// Init Typpo
let typpo = typpo_module.make();
typpo.set_target_type('ARM');
typpo.load_type_file(url.toFilename(PIEMOS_FRAMING_YAML_FILE), false);
typpo.load_type_file(url.toFilename(XBEE_FRAMING_YAML_FILE), false);
typpo.load_type_file(url.toFilename(CONFIG_YAML_FILE), false);


module.exports = typpo;
