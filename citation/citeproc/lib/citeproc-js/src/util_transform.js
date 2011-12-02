/*
 * Copyright (c) 2009 and 2010 Frank G. Bennett, Jr. All Rights
 * Reserved.
 *
 * The contents of this file are subject to the Common Public
 * Attribution License Version 1.0 (the “License”); you may not use
 * this file except in compliance with the License. You may obtain a
 * copy of the License at:
 *
 * http://bitbucket.org/fbennett/citeproc-js/src/tip/LICENSE.
 *
 * The License is based on the Mozilla Public License Version 1.1 but
 * Sections 14 and 15 have been added to cover use of software over a
 * computer network and provide for limited attribution for the
 * Original Developer. In addition, Exhibit A has been modified to be
 * consistent with Exhibit B.
 *
 * Software distributed under the License is distributed on an “AS IS”
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is the citation formatting software known as
 * "citeproc-js" (an implementation of the Citation Style Language
 * [CSL]), including the original test fixtures and software located
 * under the ./std subdirectory of the distribution archive.
 *
 * The Original Developer is not the Initial Developer and is
 * __________. If left blank, the Original Developer is the Initial
 * Developer.
 *
 * The Initial Developer of the Original Code is Frank G. Bennett,
 * Jr. All portions of the code written by Frank G. Bennett, Jr. are
 * Copyright (c) 2009 and 2010 Frank G. Bennett, Jr. All Rights Reserved.
 *
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU Affero General Public License (the [AGPLv3]
 * License), in which case the provisions of [AGPLv3] License are
 * applicable instead of those above. If you wish to allow use of your
 * version of this file only under the terms of the [AGPLv3] License
 * and not to allow others to use your version of this file under the
 * CPAL, indicate your decision by deleting the provisions above and
 * replace them with the notice and other provisions required by the
 * [AGPLv3] License. If you do not delete the provisions above, a
 * recipient may use your version of this file under either the CPAL
 * or the [AGPLv3] License.”
 */

/*
 * Fields can be transformed by translation/transliteration, or by
 * abbreviation.  Two levels of translation/transliteration
 * are available: primary-only (a one-to-one transform) or
 * primary+secondary (a transform resulting in two fields of
 * output, with implicit punctuation formatting).
 *
 * The primary+secondary transliteration/translation level is
 * available only with full-form fields.  Primary-only
 * transliteration/translation is available with both full-form
 * and short-form fields.  In this case, the abbreviation is
 * applied after the language transform.
 *
 * The transformation object here applies the most aggressive
 * transformation available under a given set of parameters.
 * It works only with simple string fields; multilingual
 * dates are handled by a separate mechanism, and numeric
 * fields are not subject to transformation.
 *
 * The transformed output is written directly to the output
 * queue.  This is necessary to cover the possibility of
 * two output fields with separate formatting requirements.
 *
 * This object itself returns an appropriate token function
 * with a standard interface, for use at runtime.
 *
 * Instantiation arguments:
 *   state object
 *
 * Initialization arguments
 *
 *   Required arguments
 *     default formatting token
 *     field name
 *
 *   Optional argument (used only if abbreviation is required)
 *     subsection
 *
 * Abbreviation
 *
 *   Optional setters:
 *     .setAbbreviationFallback(); fallback flag
 *       (if true, a failed abbreviation will fallback to long)
 *     .setAlternativeVariableName(): alternative variable name in Item,
 *       for use as a fallback abbreviation source
 *
 * Translation/transliteration
 *
 *   Required setter:
 *     .setTransformLocale(): mode (one of "default-locale", "locale-pri",
 *       "locale-sec" or "locale-sort")
 *
 *   Optional setter:
 *     .setTransformFallback():
 *       default flag (if true, the original field value will be used as a fallback)
 *
 *
 * The getTextSubField() method may be used to obtain a string transform
 * of a field, without abbreviation, as needed for setting sort keys
 * (for example).
 *
 */

CSL.Transform = function (state) {
	var debug = false, abbreviations, token, fieldname, subsection, opt;

	// Abbreviation subsections
	this["container-title"] = {};
	this["collection-title"] = {};
	this.institution = {};
	this.authority = {};
	this.title = {};
	this.publisher = {};
	this["publisher-place"] = {};
	this.hereinafter = {};

	abbreviations = "default";

	// Initialization method
	function init(t, f, x) {
		token = t;
		fieldname = f;
		subsection = x;
		opt = {
			abbreviation_fallback: false,
			alternative_varname: false,
			transform_locale: false,
			transform_fallback: false
		};
	}
	this.init = init;

	// Internal function
	function abbreviate(state, Item, altvar, basevalue, mysubsection, use_field) {
		var value;
		if (!mysubsection) {
			return basevalue;
		}
		value = "";
		if (state.transform[mysubsection]) {
			if (state.transform[mysubsection][basevalue]) {
				value = state.transform[mysubsection][basevalue];
			} else if ("string" != typeof state.transform[mysubsection][basevalue]) {
				//SNIP-START
				if (this.debug) {
					CSL.debug("UNKNOWN ABBREVIATION FOR ... " + basevalue);
				}
				//SNIP-END
				state.transform[mysubsection][basevalue] = "";
			}
		}
		if (!value && Item[altvar] && use_field) {
			value = Item[altvar];
		}
		if (!value) {
			value = basevalue;
		}
		return value;
	}

	// Internal function
	function getTextSubField(Item, field, locale_type, use_default) {
		var m, lst, opt, o, oo, pos, key, ret, len, myret, opts;
		if (!Item[field]) {
			return "";
		}
		ret = "";

		opts = state.opt[locale_type];
		if ("undefined" === typeof opts) {
			opts = state.opt["default-locale"];
		}

		for (var i = 0, ilen = opts.length; i < ilen; i += 1) {
			// Fallback from more to less specific language tag
			opt = opts[i];
			o = opt.split(/[-_]/)[0];
			if (opt && Item.multi && Item.multi._keys[field] && Item.multi._keys[field][opt]) {
				ret = Item.multi._keys[field][opt];
				break;
			} else if (o && Item.multi && Item.multi._keys[field] && Item.multi._keys[field][o]) {
				ret = Item.multi._keys[field][o];
				break;
			}
		}
		if (!ret && use_default) {
			ret = Item[field];
		}
		return ret;
	}

	//
	function setAbbreviationFallback(b) {
		opt.abbreviation_fallback = b;
	}
	this.setAbbreviationFallback = setAbbreviationFallback;

	//
	function setAlternativeVariableName(s) {
		opt.alternative_varname = s;
	}
	this.setAlternativeVariableName = setAlternativeVariableName;

	//
	function setTransformLocale(s) {
		opt.transform_locale = s;
	}
	this.setTransformLocale = setTransformLocale;

	//
	function setTransformFallback(b) {
		opt.transform_fallback = b;
	}
	this.setTransformFallback = setTransformFallback;

	// Setter for abbreviation lists
	function setAbbreviations(name) {
		var vartype, pos, len;
		if (name) {
			abbreviations = name;
		}
		len = CSL.MULTI_FIELDS.length;
		for (pos = 0; pos < len; pos += 1) {
			vartype = CSL.MULTI_FIELDS[pos];
			this[vartype] = state.sys.getAbbreviations(abbreviations, vartype);
		}
	}
	this.setAbbreviations = setAbbreviations;

	// Return function appropriate to selected options
	function getOutputFunction() {
		var mytoken, mysubsection, myfieldname, abbreviation_fallback, alternative_varname, transform_locale, transform_fallback, getTextSubfield;

		// Freeze mandatory values
		mytoken = CSL.Util.cloneToken(token); // the token isn't needed, is it?
		mysubsection = subsection;
		myfieldname = fieldname;

		// Freeze option values
		abbreviation_fallback = opt.abbreviation_fallback;
		alternative_varname = opt.alternative_varname;
		transform_locale = opt.transform_locale;
		transform_fallback = opt.transform_fallback;

		// XXXXX This is a try-and-see change, we'll see how it goes.
		// Apply uniform transforms to all variables that request
		// translation.
		if (false && mysubsection) {
			// Short form
			return function (state, Item) {
				var primary;

				primary = getTextSubField(Item, myfieldname, transform_locale, transform_fallback);
				primary = abbreviate(state, Item, alternative_varname, primary, mysubsection, true);
				state.output.append(primary, this);
			};
		} else if (transform_locale === "locale-sec") {
			// Long form, with secondary translation
			return function (state, Item) {
				var primary, secondary, primary_tok, secondary_tok, key;
				if (state.opt["locale-suppress-title-transliteration"] 
					|| !((state.tmp.area === 'bibliography'
						|| (state.opt.xclass === "note" &&
							state.tmp.area === "citation"))
						)
					) {
					primary = Item[myfieldname];
				} else {
					primary = getTextSubField(Item, myfieldname, "locale-pri", transform_fallback);
				}
				// Signifying short form -- the variable name is misleading.
				if (mysubsection) {
					primary = abbreviate(state, Item, alternative_varname, primary, mysubsection, true);
				}
				secondary = getTextSubField(Item, myfieldname, "locale-sec");
				if (secondary && ((state.tmp.area === 'bibliography' || (state.opt.xclass === "note" && state.tmp.area === "citation")))) {
					// Signifying short form -- again, the variable name is misleading.
					if (mysubsection) {
						secondary = abbreviate(state, Item, alternative_varname, secondary, mysubsection, true);
					}
					primary_tok = CSL.Util.cloneToken(this);
					primary_tok.strings.suffix = "";
					secondary_tok = new CSL.Token("text", CSL.SINGLETON);
					secondary_tok.strings.suffix = "]" + this.strings.suffix;
					secondary_tok.strings.prefix = " [";
					
					state.output.append(primary, primary_tok);
					state.output.append(secondary, secondary_tok);
				} else {
					state.output.append(primary, this);
				}
				return null;
			};
		} else {
			return function (state, Item) {
				var primary;
				primary = getTextSubField(Item, myfieldname, transform_locale, transform_fallback);
				state.output.append(primary, this);
				return null;
			};
		}
	}
	this.getOutputFunction = getOutputFunction;

	function output(state, basevalue, token_short, token_long, use_fallback) {
		//
		// This output method is specific to institutions.
		// See util_institutions.js
		//
		var shortvalue;
		//
		// This was pointless: institutions are names, and language
		// selection is done with this.name().
		//basevalue = this.getTextSubField(value, "locale-pri", true);

		shortvalue = state.transform.institution[basevalue];
		if (shortvalue) {
			state.output.append(shortvalue, token_short);
		} else {
			if (use_fallback) {
				state.output.append(basevalue, token_long);
			}
			//SNIP-START
			if (this.debug) {
				CSL.debug("UNKNOWN ABBREVIATION FOR: " + basevalue);
			}
			//SNIP-END
		}
	}
	this.output = output;

	function getStaticOrder (name, refresh) {
		var static_ordering_val = false;
		if (!refresh && name["static-ordering"]) {
			static_ordering_val = true;
		} else if (!(name.family.replace('"', '', 'g') + name.given).match(CSL.ROMANESQUE_REGEXP)) {
			static_ordering_val = true;
		} else if (name.multi && name.multi.main && name.multi.main.slice(0,2) == 'vn') {
			static_ordering_val = true;
		} else {
			if (state.opt['auto-vietnamese-names']
				&& (CSL.VIETNAMESE_NAMES.exec(name.family + " " + name.given)
					&& CSL.VIETNAMESE_SPECIALS.exec(name.family + name.given))) {

				static_ordering_val = true;
			}
		}
		return static_ordering_val;
	}

	// The name transform code is placed here to keep similar things
	// in one place.  Obviously this module could do with a little
	// tidying up.

	/*
	 * Return a single name object
	 */
	function name (state, name, langTags) {
		var i, ret, optLangTag, ilen, key, langTag;
		if (state.tmp.area.slice(-5) === "_sort") {
			 langTags = state.opt["locale-sort"];
		}
		if ("string" === typeof langTags) {
			langTags = [langTags];
		}
		// Normalize to string
		if (!name.family) {
			name.family = "";
		}
		if (!name.given) {
			name.given = "";
		}
		//
		// Optionally add a static-ordering toggle for non-roman, non-Cyrillic
		// names, based on the headline values.
		//
		var static_ordering_freshcheck = false;
		var block_initialize = false;
		var transliterated = false;
		var static_ordering_val = getStaticOrder(name);
		//
		// Step through the requested languages in sequence
		// until a match is found
		//
		if (langTags && name.multi) {
			for (i = 0, ilen = langTags.length; i < ilen; i += 1) {
				langTag = langTags[i];
				if (name.multi._key[langTag]) {
					name = name.multi._key[langTag];
					transliterated = true;
					if (!state.opt['locale-use-original-name-format']) {
						static_ordering_freshcheck = true;
					} else {
						// Quash initialize-with if original was non-romanesque
						// and we are trying to preserve the original formatting
						// conventions.
						// (i.e. supply as much information as possible if
						// the transliteration spans radically different
						// writing conventions)
						if ((name.family.replace('"','','g') + name.given).match(CSL.ROMANESQUE_REGEXP)) {
							block_initialize = true;
						}
					}
					break;
				}
			}
		}
		// var clone the item before writing into it
		name = {
			family:name.family,
			given:name.given,
			"non-dropping-particle":name["non-dropping-particle"],
			"dropping-particle":name["dropping-particle"],
			suffix:name.suffix,
			"static-ordering":static_ordering_val,
			"parse-names":name["parse-names"],
			"comma-suffix":name["comma-suffix"],
			transliterated:transliterated,
			block_initialize:block_initialize
		}
		if (static_ordering_freshcheck &&
			!getStaticOrder(name, true)) {
			
			name["static-ordering"] = false;
		}
		if (state.opt["parse-names"]
			&& name["parse-names"] !== 0) {
			state.parseName(name);
		}
		if (name.family && name.family.length && name.family.slice(0, 1) === '"' && name.family.slice(-1) === '"') {
			name.family = name.family.slice(1, -1);
		}
		if (!name.literal && !name.given && name.family) {
			name.literal = name.family;
		}
		if (name.literal) {
			delete name.family;
			delete name.given;
		}
		return name;
	}
	this.name = name;
};



