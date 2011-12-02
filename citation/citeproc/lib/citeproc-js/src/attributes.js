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

CSL.Attributes = {};

CSL.Attributes["@class"] = function (state, arg) {
	state.opt["class"] = arg;
};

CSL.Attributes["@version"] = function (state, arg) {
	state.opt.version = arg;
};

/**
 * Store the value attribute on the token.
 * @name CSL.Attributes.@value
 * @function
 */
CSL.Attributes["@value"] = function (state, arg) {
	this.strings.value = arg;
};


/**
 * Store the name attribute (of a macro or term node)
 * on the state object.
 * <p>For reference when the closing node of a macro
 * or locale definition is encountered.</p>
 * @name CSL.Attributes.@name
 * @function
 */
CSL.Attributes["@name"] = function (state, arg) {
	this.strings.name = arg;
};


/**
 * Store the form attribute (of a term node) on the state object.
 * <p>For reference when the closing node of a macro
 * or locale definition is encountered.</p>
 * @name CSL.Attributes.@form
 * @function
 */
CSL.Attributes["@form"] = function (state, arg) {
	this.strings.form = arg;
};

CSL.Attributes["@date-parts"] = function (state, arg) {
	this.strings["date-parts"] = arg;
};

CSL.Attributes["@range-delimiter"] = function (state, arg) {
	this.strings["range-delimiter"] = arg;
};

/**
 * Store macro tokens in a buffer on the state object.
 * <p>For reference when the enclosing text token is
 * processed.</p>
 * @name CSL.Attributes.@macro
 * @function
 */
CSL.Attributes["@macro"] = function (state, arg) {
	this.postponed_macro = arg;
};


CSL.Attributes["@term"] = function (state, arg) {
	if (this.name === "et-al") {
		if (state.locale[state.opt.lang].terms[arg]) {
			this.strings.term = state.getTerm(arg, "long", 0);
		} else {
			this.strings.term = arg;
		}
	} else {
		state.build.term = arg;
	}
};


/*
 * Ignore xmlns attribute.
 * <p>This should always be <p>http://purl.org/net/xbiblio/csl</code>
 * anyway.  At least for the present we will blindly assume
 * that it is.</p>
 * @name CSL.Attributes.@xmlns
 * @function
 */
CSL.Attributes["@xmlns"] = function (state, arg) {};


/*
 * Store language attribute to a buffer field.
 * <p>Will be placed in the appropriate location
 * when the element is processed.</p>
 * @name CSL.Attributes.@lang
 * @function
 */
CSL.Attributes["@lang"] = function (state, arg) {
	if (arg) {
		state.build.lang = arg;
	}
};


/*
 * Store item type evaluation function on token.
 * @name CSL.Attributes.@type
 * @function
 */
CSL.Attributes["@type"] = function (state, arg) {
	var types, ret, func, len, pos;
	func = function (state, Item) {
		types = arg.split(/\s+/);
		ret = [];
		len = types.length;
		for (pos = 0; pos < len; pos += 1) {
			ret.push(Item.type === types[pos]);
		}
		return ret;
	};
	this.tests.push(func);
};


/*
 * Store variable retrieval function on token.
 * <p>Returned function will return the variable value
 * or values as an array.</p>
 * @name CSL.Attributes.@variable
 * @function
 */
CSL.Attributes["@variable"] = function (state, arg) {
	var variables, pos, len, func, output, variable, varlen, needlen, ret, x, myitem, key, flag;
	this.variables = arg.split(/\s+/);
	this.variables_real = arg.split(/\s+/);
	if ("label" === this.name && this.variables[0]) {
		state.build.term = this.variables[0];
	} else if (["names", "date", "text", "number"].indexOf(this.name) > -1) {
		//
		// An oddity of variable handling is that this.variables
		// is actually ephemeral; the full list of variables is
		// held in the variables_real var, and pushed into this.variables
		// conditionally in order to suppress repeat renderings of
		// the same item variable.  [STILL FUNCTIONAL? 2010.01.15]
		//
		// set variable names
		func = function (state, Item) {
			variables = this.variables_real.slice();
			this.variables = [];

			len = variables.length;
			for (pos = 0; pos < len; pos += 1) {
				if (state.tmp.done_vars.indexOf(variables[pos]) === -1) {
					this.variables.push(variables[pos]);
				}
				if (state.tmp.can_block_substitute) {
					state.tmp.done_vars.push(variables[pos]);
				}
			}
		};
		this.execs.push(func);

		// check for output
		func = function (state, Item, item) {
			var mydate;
			output = false;
			len = this.variables.length;
			for (pos = 0; pos < len; pos += 1) {
				variable = this.variables[pos];
				if (this.strings.form === "short" && !Item[variable]) {
					if (variable === "title") {
						variable = "shortTitle";
					} else if (variable === "container-title") {
						variable = "journalAbbreviation";
					} else if (variable === "page-first") {
						variable = "page";
					}
				}
				if (CSL.DATE_VARIABLES.indexOf(variable) > -1) {
					if (Item[variable] && Item[variable].raw) {
						output = true;
						break;
					} else if (Item[variable] && Item[variable].literal) {
						output = true;
						break;
					} else if (Item[variable] && Item[variable]['date-parts'] && Item[variable]['date-parts'].length && this.dateparts && this.dateparts.length) {
						varlen = Item[variable]['date-parts'][0].length;
						needlen = 4;
						if (this.dateparts.indexOf("year") > -1) {
							needlen = 1;
						} else if (this.dateparts.indexOf("month") > -1) {
							needlen = 2;
						} else if (this.dateparts.indexOf('day') > -1) {
							needlen = 3;
						}
						if (varlen >= needlen) {
							output = true;
							break;
						}
					}
				} else if ("locator" === variable) {
					if (item && item.locator) {
						output = true;
						break;
					}
				} else if ("citation-number" === variable) {
					output = true;
					break;
				} else if ("first-reference-note-number" === variable) {
					if (item && item["first-reference-note-number"]) {
						output = true;
						break;
					}
				} else if ("object" === typeof Item[variable]) {
					if (Item[variable].length) {
						output = true;
						break;
					}
				} else if ("string" === typeof Item[variable] && Item[variable]) {
					output = true;
					break;
				} else if ("number" === typeof Item[variable]) {
					output = true;
					break;
				}
				if (output) {
					break;
				}
			}
			//print("-- VAR: "+variable);
			flag = state.tmp.term_sibling.value();
			if (output) {
				if (variable !== "citation-number" || !state.tmp.area === "bibliography") {
					state.tmp.cite_renders_content = true;
				}
				flag[2] = true;
				state.tmp.term_sibling.replace(flag);
				state.tmp.can_substitute.replace(false,  CSL.LITERAL);
			} else {
				flag[1] = true;
			}
		};
		this.execs.push(func);
	} else if (["if",  "else-if"].indexOf(this.name) > -1) {
		// check for variable value
		func = function (state, Item, item) {
			var key;
			ret = [];
			len = this.variables.length;
			for (pos = 0; pos < len; pos += 1) {
				variable = this.variables[pos];
				x = false;
				myitem = Item;
				if (item && ["locator", "first-reference-note-number"].indexOf(variable) > -1) {
					myitem = item;
				}
				if (myitem[variable]) {
					if ("number" === typeof myitem[variable] || "string" === typeof myitem[variable]) {
						x = true;
					} else if ("object" === typeof myitem[variable]) {
						//
						// this will turn true only for hash objects
						// that have at least one attribute, or for a
						// non-zero-length list
						//
						for (key in myitem[variable]) {
							if (myitem[variable].hasOwnProperty(key)) {
								x = true;
								break;
							}
						}
					}
				}
				ret.push(x);
			}
			return ret;
		};
		this.tests.push(func);
	}
};

// Used as a flag during dates processing
CSL.Attributes["@lingo"] = function (state, arg) {
}

CSL.Attributes["@locale"] = function (state, arg) {
	var func, ret, len, pos, variable, myitem, langspec, lang, lst, i, ilen, fallback;

	if (this.name === "layout") {
		// For layout
		this.locale_raw = arg;
	} else {
		// For if and if-else

		// Split argument
		lst = arg.split(/\s+/);

		// Expand each list element
		this.locale_bases = [];
		for (i = 0, ilen = lst.length; i < ilen; i += 1) {
			// Parse out language string
			lang = CSL.localeParse(lst[i]);
		
			// Analyze the locale
			langspec = CSL.localeResolve(lang);
			if (lst[i].length === 2) {
				// For fallback
				this.locale_bases.push(langspec.base);
			}
			// Load the locale terms etc.
			state.localeConfigure(langspec);
			
			// Replace string with locale spec object
			lst[i] = langspec;
		}
		// Set locale tag on node
		this.locale_default = state.opt["default-locale"][0];
		// The locale to set on node children if match is successful
		this.locale = lst[0].best;
		// Locales to test
		this.locale_list = lst.slice();
		
		// check for variable value
		func = function (state, Item, item) {
			var key, res;
			ret = [];
			if (Item.language) {
				lang = CSL.localeParse(Item.language);
				langspec = CSL.localeResolve(lang);
				// We attempt to match a specific locale from the
				// list of parameters.  If that fails, we fall back
				// to the base locale of the first element.  The
				// locale applied is always the first local 
				// in the list of parameters (or base locale, for a 
				// single two-character language code) 
				res = false;
				for (i = 0, ilen = this.locale_list.length; i < ilen; i += 1) {
					if (langspec.best === this.locale_list[i].best) {
						state.opt.lang = this.locale;
						state.tmp.last_cite_locale = this.locale;
						// Set empty group open tag with locale set marker
						state.output.openLevel("empty");
						state.output.current.value().new_locale = this.locale;
						res = true;
						break;
					}
				}
				if (!res && this.locale_bases.indexOf(langspec.base) > -1) {
					state.opt.lang = this.locale;
					state.tmp.last_cite_locale = this.locale;
					// Set empty group open tag with locale set marker
					state.output.openLevel("empty");
					state.output.current.value().new_locale = this.locale;
					res = true;
				}
			}
			ret.push(res);
			return ret;
		};
		this.tests.push(func);
	};
};

/*
 * Store suffix string on token.
 * @name CSL.Attributes.@suffix
 * @function
 */
CSL.Attributes["@suffix"] = function (state, arg) {
	this.strings.suffix = arg;
};


/*
 * Store prefix string on token.
 * @name CSL.Attributes.@prefix
 * @function
 */
CSL.Attributes["@prefix"] = function (state, arg) {
	this.strings.prefix = arg;
};


/*
 * Store delimiter string on token.
 * @name CSL.Attributes.@delimiter
 * @function
 */
CSL.Attributes["@delimiter"] = function (state, arg) {
	this.strings.delimiter = arg;
};


/*
 * Store match evaluator on token.
 */
CSL.Attributes["@match"] = function (state, arg) {
	var evaluator;
	if (this.tokentype === CSL.START || CSL.SINGLETON) {
		if ("none" === arg) {
			evaluator = state.fun.match.none;
		} else if ("any" === arg) {
			evaluator = state.fun.match.any;
		} else if ("all" === arg) {
			evaluator = state.fun.match.all;
		} else {
			throw "Unknown match condition \"" + arg + "\" in @match";
		}
		this.evaluator = evaluator;
	}
};

CSL.Attributes["@is-uncertain-date"] = function (state, arg) {
	var variables, len, pos, func, variable, ret;
	variables = arg.split(/\s+/);
	len = variables.length;
	func = function (state, Item) {
		ret = [];
		for (pos = 0; pos < len; pos += 1) {
			variable = variables[pos];
			if (Item[variable] && Item[variable].circa) {
				ret.push(true);
			} else {
				ret.push(false);
			}
		}
		return ret;
	};
	this.tests.push(func);
};

CSL.Attributes["@is-numeric"] = function (state, arg) {
	var variables, variable, func, val, pos, len, not_numeric_type, ret;
	variables = arg.split(/\s+/);
	len = variables.length;
	func = function (state, Item) {
		ret = [];
		for (pos = 0; pos < len; pos += 1) {
			variable = variables[pos];
			not_numeric_type = CSL.NUMERIC_VARIABLES.indexOf(variable) === -1;
			val = Item[variable];
			if (typeof val === "number") {
				val = val.toString();
			}

			if (not_numeric_type) {
				if (Item[variable] && ("" + Item[variable]).match(/[0-9]$/)) {
					ret.push(true);
				} else {
					ret.push(false);
				}
			} else  if (typeof val === "undefined") {
				ret.push(false);
			} else if (typeof val !== "string") {
				ret.push(false);
			} else if (val.match(CSL.QUOTED_REGEXP_START) && val.match(CSL.QUOTED_REGEXP_END)) {
				ret.push(false);
			} else if (val.match(CSL.NUMBER_REGEXP)) {
				ret.push(true);
			} else {
				ret.push(false);
			}
		}
		return ret;
	};
	this.tests.push(func);
};


CSL.Attributes["@names-min"] = function (state, arg) {
	var val = parseInt(arg, 10);
	if (state.opt.max_number_of_names < val) {
		state.opt.max_number_of_names = val;
	}
	this.strings["et-al-min"] = val;
};

CSL.Attributes["@names-use-first"] = function (state, arg) {
	this.strings["et-al-use-first"] = parseInt(arg, 10);
};

CSL.Attributes["@names-use-last"] = function (state, arg) {
    	if (arg === "true") {
		this.strings["et-al-use-last"] = true;
	} else {
		this.strings["et-al-use-last"] = false;
	}
};

CSL.Attributes["@sort"] = function (state, arg) {
	if (arg === "descending") {
		this.strings.sort_direction = CSL.DESCENDING;
	}
};


CSL.Attributes["@plural"] = function (state, arg) {
	// Accepted values of plural attribute differ on cs:text
	// and cs:label nodes.
	if ("always" === arg || "true" === arg) {
		this.strings.plural = 1;
	} else if ("never" === arg || "false" === arg) {
		this.strings.plural = 0;
	} else if ("contextual" === arg) {
		this.strings.plural = false;
	}
};


CSL.Attributes["@locator"] = function (state, arg) {
	var func;
	if (["if",  "else-if"].indexOf(this.name) > -1) {
		// check for variable value
		func = function (state, Item, item) {
			var label;
			if ("undefined" === typeof item || !item.label) {
				label = "page";
			} else {
				label = item.label;
			}
			if (arg === label) {
				return true;
			}
			return false;
		};
		this.tests.push(func);
	}
};


CSL.Attributes["@newdate"] = function (state, arg) {

};


CSL.Attributes["@position"] = function (state, arg) {
    var tryposition;
	state.opt.update_mode = CSL.POSITION;
	var lst = arg.split(/\s+/);
	for (var i = 0, ilen = lst.length; i < ilen; i += 1) {
	    if (state.build.area.slice(0, 12) === "bibliography") {
		func = function (state, Item, item) {
		    return false;
		}
		this.tests.push(func);
	    } else {
		if (lst[i] === "first") {
		    tryposition = CSL.POSITION_FIRST;
		} else if (lst[i] === "subsequent") {
		    tryposition = CSL.POSITION_SUBSEQUENT;
		} else if (lst[i] === "ibid") {
		    tryposition = CSL.POSITION_IBID;
		} else if (lst[i] === "ibid-with-locator") {
		    tryposition = CSL.POSITION_IBID_WITH_LOCATOR;
		}
		// ZZZZZ We need a factory function here, similar
		// to what we do for decorations.
		var factory = function (tryposition) {
		    return  function (state, Item, item) {
			if (item && "undefined" === typeof item.position) {
			    item.position = 0;
			}
			if (item && typeof item.position === "number") {
			    if (item.position === 0 && tryposition === 0) {
				return true;
			    } else if (tryposition > 0 && item.position >= tryposition) {
				return true;
			    }
			} else if (tryposition === 0) {
			    return true;
			}
			return false;
		    };
		};
		func = factory(tryposition);
		this.tests.push(func);
	    }
	    if (lst[i] === "near-note") {
		func = function (state, Item, item) {
		    if (item && item["near-note"]) {
			return true;
		    }
		    return false;
		};
		this.tests.push(func);
	    }
	}
};


CSL.Attributes["@disambiguate"] = function (state, arg) {
	if (this.tokentype === CSL.START && ["if", "else-if"].indexOf(this.name) > -1) {
		if (arg === "true") {
			state.opt.has_disambiguate = true;
			var func = function (state, Item) {
				if (state.tmp.disambig_settings.disambiguate) {
					return true;
				}
				return false;
			};
			this.tests.push(func);
		}
	}
};

CSL.Attributes["@givenname-disambiguation-rule"] = function (state, arg) {
	if (CSL.GIVENNAME_DISAMBIGUATION_RULES.indexOf(arg) > -1) {
		state.opt["givenname-disambiguation-rule"] = arg;
	}
};

CSL.Attributes["@collapse"] = function (state, arg) {
	// only one collapse value will be honoured.
	if (arg) {
		state[this.name].opt.collapse = arg;
	}
};



CSL.Attributes["@names-delimiter"] = function (state, arg) {
	state.setOpt(this, "names-delimiter", arg);
};

CSL.Attributes["@name-form"] = function (state, arg) {
	state.setOpt(this, "name-form", arg);
};

CSL.Attributes["@name-delimiter"] = function (state, arg) {
	state.setOpt(this, "name-delimiter", arg);
};

CSL.Attributes["@et-al-min"] = function (state, arg) {
	var val = parseInt(arg, 10);
	if (state.opt.max_number_of_names < val) {
		state.opt.max_number_of_names = val;
	}
	state.setOpt(this, "et-al-min", val);
};

CSL.Attributes["@et-al-use-first"] = function (state, arg) {
	state.setOpt(this, "et-al-use-first", parseInt(arg));
};

CSL.Attributes["@et-al-use-last"] = function (state, arg) {
	if (arg === "true") {
		state.setOpt(this, "et-al-use-last", true);
	} else {
		state.setOpt(this, "et-al-use-last", false);
	}
};

CSL.Attributes["@et-al-subsequent-min"] = function (state, arg) {
	state.opt.update_mode = CSL.POSITION;
	var val = parseInt(arg, 10);
	if (state.opt.max_number_of_names < val) {
		state.opt.max_number_of_names = val;
	}
	state.setOpt(this, "et-al-subsequent-min", val);
};

CSL.Attributes["@et-al-subsequent-use-first"] = function (state, arg) {
	state.opt.update_mode = CSL.POSITION;
	state.setOpt(this, "et-al-subsequent-use-first", parseInt(arg, 10));
};

CSL.Attributes["@truncate-min"] = function (state, arg) {
	this.strings["truncate-min"] = parseInt(arg, 10);
};

CSL.Attributes["@suppress-min"] = function (state, arg) {
	this.strings["suppress-min"] = parseInt(arg, 10);
};


CSL.Attributes["@and"] = function (state, arg) {
	var myarg, and;
	myarg = "&";
	if ("text" === arg) {
		and = state.getTerm("and", "long", 0);
		myarg = and;
	}
	state.setOpt(this, "and", myarg);
};

CSL.Attributes["@delimiter-precedes-last"] = function (state, arg) {
	state.setOpt(this, "delimiter-precedes-last", arg);
};

CSL.Attributes["@delimiter-precedes-et-al"] = function (state, arg) {
	state.setOpt(this, "delimiter-precedes-et-al", arg);
};

CSL.Attributes["@initialize-with"] = function (state, arg) {
	state.setOpt(this, "initialize-with", arg);
};

CSL.Attributes["@name-as-sort-order"] = function (state, arg) {
	state.setOpt(this, "name-as-sort-order", arg);
};

CSL.Attributes["@sort-separator"] = function (state, arg) {
	state.setOpt(this, "sort-separator", arg);
};



CSL.Attributes["@year-suffix-delimiter"] = function (state, arg) {
	state[this.name].opt["year-suffix-delimiter"] = arg;
};

CSL.Attributes["@after-collapse-delimiter"] = function (state, arg) {
	state[this.name].opt["after-collapse-delimiter"] = arg;
};

CSL.Attributes["@subsequent-author-substitute"] = function (state, arg) {
	state[this.name].opt["subsequent-author-substitute"] = arg;
};

CSL.Attributes["@disambiguate-add-names"] = function (state, arg) {
	if (arg === "true") {
		state.opt["disambiguate-add-names"] = true;
	}
};

CSL.Attributes["@disambiguate-add-givenname"] = function (state, arg) {
	if (arg === "true") {
		state.opt["disambiguate-add-givenname"] = true;
	}
};

CSL.Attributes["@disambiguate-add-year-suffix"] = function (state, arg) {
	if (arg === "true") {
		state.opt["disambiguate-add-year-suffix"] = true;
	}
};


CSL.Attributes["@second-field-align"] = function (state, arg) {
	if (arg === "flush" || arg === "margin") {
		state[this.name].opt["second-field-align"] = arg;
	}
};


CSL.Attributes["@hanging-indent"] = function (state, arg) {
	if (arg === "true") {
		state[this.name].opt.hangingindent = 2;
	}
};


CSL.Attributes["@line-spacing"] = function (state, arg) {
	if (arg && arg.match(/^[.0-9]+$/)) {
		state[this.name].opt["line-spacing"] = parseFloat(arg, 10);
	}
};


CSL.Attributes["@entry-spacing"] = function (state, arg) {
	if (arg && arg.match(/^[.0-9]+$/)) {
		state[this.name].opt["entry-spacing"] = parseFloat(arg, 10);
	}
};


CSL.Attributes["@near-note-distance"] = function (state, arg) {
	state[this.name].opt["near-note-distance"] = parseInt(arg, 10);
};

CSL.Attributes["@page-range-format"] = function (state, arg) {
	state.opt["page-range-format"] = arg;
};

CSL.Attributes["@text-case"] = function (state, arg) {
	this.strings["text-case"] = arg;
};


CSL.Attributes["@page-range-format"] = function (state, arg) {
	state.opt["page-range-format"] = arg;
};


CSL.Attributes["@default-locale"] = function (state, arg) {
	var lst, len, pos, m, ret;
	//
	// Workaround for Internet Exploder 6 (doesn't recognize
	// groups in str.split(/something(braced-group)something/)
	//
	m = arg.match(/-x-(sort|pri|sec|name)-/g);
	if (m) {
		for (pos = 0, len = m.length; pos < len; pos += 1) {
			m[pos] = m[pos].replace(/^-x-/, "").replace(/-$/, "");
		}
	}
	lst = arg.split(/-x-(?:sort|pri|sec|name)-/);
	ret = [lst[0]];
	for (pos = 1, len = lst.length; pos < len; pos += 1) {
		ret.push(m[pos - 1]);
		ret.push(lst[pos]);
	}
	lst = ret.slice();
	len = lst.length;
	for (pos = 1; pos < len; pos += 2) {
		state.opt[("locale-" + lst[pos])].push(lst[(pos + 1)].replace(/^\s*/g, "").replace(/\s*$/g, ""));
	}
	if (lst.length) {
		state.opt["default-locale"] = lst.slice(0, 1);
	} else {
		state.opt["default-locale"] = ["en"];
	}
};

CSL.Attributes["@demote-non-dropping-particle"] = function (state, arg) {
	state.opt["demote-non-dropping-particle"] = arg;
};

CSL.Attributes["@initialize-with-hyphen"] = function (state, arg) {
	if (arg === "false") {
		state.opt["initialize-with-hyphen"] = false;
	}
};

CSL.Attributes["@institution-parts"] = function (state, arg) {
	this.strings["institution-parts"] = arg;
};

CSL.Attributes["@if-short"] = function (state, arg) {
	if (arg === "true") {
		this.strings["if-short"] = true;
	}
};

CSL.Attributes["@substitute-use-first"] = function (state, arg) {
	this.strings["substitute-use-first"] = parseInt(arg, 10);
};

CSL.Attributes["@use-first"] = function (state, arg) {
	this.strings["use-first"] = parseInt(arg, 10);
};

CSL.Attributes["@use-last"] = function (state, arg) {
	if (arg.match(/^[0-9]+$/)) {
		this.strings["use-last"] = parseInt(arg, 10);
	}
};


CSL.Attributes["@reverse-order"] = function (state, arg) {
	if ("true" === arg) {
		this.strings["reverse-order"] = true;
	}
};

CSL.Attributes["@display"] = function (state, arg) {
	this.strings.cls = arg;
};

