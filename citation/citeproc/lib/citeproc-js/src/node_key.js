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

CSL.Node.key = {
	build: function (state, target) {
		var start_key, func, sort_direction, variable, names_start_token, name_token, names_end_token, single_text, token, pos, keypos, ppos, len, llen, tok, gtok, saveme, end_key, tlen, tlst, debug;
		debug = false;
		start_key = new CSL.Token("key", CSL.START);
		start_key.strings["et-al-min"] = this.strings["et-al-min"];
		start_key.strings["et-al-use-first"] = this.strings["et-al-use-first"];
		start_key.strings["et-al-use-last"] = this.strings["et-al-use-last"];
		// initialize done vars
		func = function (state, Item) {
			state.tmp.done_vars = [];
		};
		start_key.execs.push(func);

		// sort direction
		sort_direction = [];
		if (this.strings.sort_direction === CSL.DESCENDING) {
			sort_direction.push(1);
			sort_direction.push(-1);
		} else {
			sort_direction.push(-1);
			sort_direction.push(1);
		}
		state[state.build.area].opt.sort_directions.push(sort_direction);

		// et al init
		func = function (state, Item) {
			state.tmp.sort_key_flag = true;
			if (this.strings["et-al-min"]) {
				state.tmp["et-al-min"] = this.strings["et-al-min"];
			}
			if (this.strings["et-al-use-first"]) {
				state.tmp["et-al-use-first"] = this.strings["et-al-use-first"];
			}
			if (this.strings["et-al-use-last"]) {
				state.tmp["et-al-use-last"] = this.strings["et-al-use-last"];
			}
		};
		start_key.execs.push(func);
		target.push(start_key);
		//
		// ops to initialize the key's output structures
		if (this.variables.length) {
			variable = this.variables[0];
			// Set flag if sorting citations by citation-number
			// XXXXX: This will assume citation-number sorting if
			// that variable is set as key in ANY position.  Could
			// be a little more conservative, but secondary sorts
			// by this variable seem unlikely.
			if (variable === "citation-number" && state.build.area === "citation_sort") {
				state.opt.citation_number_sort = true;
			}
			if (CSL.CREATORS.indexOf(variable) > -1) {
				//
				// Start tag
				names_start_token = new CSL.Token("names", CSL.START);
				names_start_token.tokentype = CSL.START;
				names_start_token.variables = this.variables;
				CSL.Node.names.build.call(names_start_token, state, target);
				//
				// Middle tag
				name_token = new CSL.Token("name", CSL.SINGLETON);
				name_token.tokentype = CSL.SINGLETON;
				name_token.strings["name-as-sort-order"] = "all";
				CSL.Node.name.build.call(name_token, state, target);
				//
				// End tag
				names_end_token = new CSL.Token("names", CSL.END);
				names_end_token.tokentype = CSL.END;
				CSL.Node.names.build.call(names_end_token, state, target);
			} else {
				single_text = new CSL.Token("text", CSL.SINGLETON);
				single_text.dateparts = this.dateparts;
				if (CSL.NUMERIC_VARIABLES.indexOf(variable) > -1) {
					func = function (state, Item) {
						var num, m;
						num = false;
						if ("citation-number" === variable) {
							num = state.registry.registry[Item.id].seq.toString();
						} else {
							num = Item[variable];
						}
						if (num) {
							// Code currently in util_number.js
							num = CSL.Util.padding(num);
						}
						state.output.append(num, this);
					};
				} else if (CSL.DATE_VARIABLES.indexOf(variable) > -1) {
					func = function (state, Item) {
						var dp, elem, value, e, yr, prefix;
						dp = Item[variable];
						if ("undefined" === typeof dp) {
							dp = {"date-parts": [[0]] };
							if (!dp.year) {
								state.tmp.empty_date = true;
							}
						}
						if ("undefined" === typeof this.dateparts) {
							this.dateparts = ["year", "month", "day"];
						}
						if (dp.raw) {
							dp = state.fun.dateparser.parse(dp.raw);
						} else if (dp["date-parts"]) {
							dp = state.dateParseArray(dp);
						}
						if ("undefined" === typeof dp) {
							dp = {};
						}
						for (var i = 0, ilen = CSL.DATE_PARTS_INTERNAL.length; i < ilen; i += 1) {
							elem = CSL.DATE_PARTS_INTERNAL[i];
							value = 0;
							e = elem;
							if (e.slice(-4) === "_end") {
								e = e.slice(0, -4);
							}
							if (dp[elem] && this.dateparts.indexOf(e) > -1) {
								value = dp[elem];
							}
							if (elem.slice(0, 4) === "year") {
								yr = CSL.Util.Dates[e].numeric(state, value);
								prefix = "Y";
								if (yr[0] === "-") {
									prefix = "X";
									yr = yr.slice(1);
									yr = 9999 - parseInt(yr, 10);
								}
								state.output.append(CSL.Util.Dates[elem.slice(0, 4)].numeric(state, (prefix + yr)));
							} else {
								state.output.append(CSL.Util.Dates[e]["numeric-leading-zeros"](state, value));
							}
						}
						if (state.registry.registry[Item.id] && state.registry.registry[Item.id].disambig.year_suffix) {
							num = state.registry.registry[Item.id].disambig.year_suffix.toString();
							num = CSL.Util.padding(num);
						} else {
							num = CSL.Util.padding("0");
						}
						state.output.append("S"+num)

					};
				} else if ("title" === variable) {
					state.transform.init("empty", "title");
					state.transform.setTransformLocale("locale-sort");
					state.transform.setTransformFallback(true);
					func = state.transform.getOutputFunction();
					//func = function (state, Item) {
					//	var value = Item[variable];
					//	if (value) {
					//		value = state.transform.getTextSubField(value, "locale-sort", true);
					//		state.output.append(value, "empty");
					//	}
					//};
				} else {
					func = function (state, Item) {
						var varval = Item[variable];
						state.output.append(varval, "empty");
					};
				}
				single_text.execs.push(func);
				target.push(single_text);
			}
		} else { // macro
			//
			// if it's not a variable, it's a macro
			token = new CSL.Token("text", CSL.SINGLETON);
			token.postponed_macro = this.postponed_macro;
			// careful with the loop below: she's sensitive
			// to change
			tlen = target.length;
			keypos = false;
			CSL.expandMacro.call(state, token);
			for (var i = 0, ilen = target.slice(tlen).length; i < ilen; i += 1) {
				tok = target.slice(tlen)[i];
				if (tok && tok.name === "text" && tok.dateparts) {
					keypos = i;
					break;
				}
			}
			if (keypos) {
				saveme = target[(parseInt(keypos, 10) + parseInt(tlen, 10))];
				len = target.length - 1;
				for (pos = len; pos > tlen; pos += -1) {
					target.pop();
				}
				target.push(saveme);
				gtok = new CSL.Token("group", CSL.END);
				target.push(gtok);
			}
		}
		//
		// ops to output the key string result to an array go
		// on the closing "key" tag before it is pushed.
		// Do not close the level.
		end_key = new CSL.Token("key", CSL.END);
		// store key for use
		func = function (state, Item) {
			var keystring = state.output.string(state, state.output.queue);
			//SNIP-START
			if (debug) {
				CSL.debug("keystring: " + keystring + " " + typeof keystring);
			}
			//SNIP-END
			if ("string" !== typeof keystring || state.tmp.empty_date) {
				keystring = undefined;
				state.tmp.empty_date = false;
			}
			state[state.tmp.area].keys.push(keystring);
			state.tmp.value = [];
		};
		end_key.execs.push(func);
		// reset key params
		func = function (state, Item) {
			// state.tmp.name_quash = new Object();
			state.tmp["et-al-min"] = false;
			state.tmp["et-al-use-first"] = false;
			state.tmp["et-al-use-last"] = false;
			state.tmp.sort_key_flag = false;
		};
		end_key.execs.push(func);
		target.push(end_key);
	}
};
