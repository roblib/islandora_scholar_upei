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

CSL.Node.text = {
	build: function (state, target) {
		var variable, func, form, plural, id, num, number, formatter, firstoutput, specialdelimiter, label, myname, names, name, year, suffix, term, dp, len, pos, n, m, value, flag;
		CSL.Util.substituteStart.call(this, state, target);
		if (this.postponed_macro) {
			CSL.expandMacro.call(state, this);
		} else {
			// ...
			//
			// Do non-macro stuff
			variable = this.variables[0];
			//if (variable) {
			//	func = function (state, Item) {
			//		state.parallel.StartVariable(this.variables[0]);
			//		state.parallel.AppendToVariable(Item[this.variables[0]]);
			//	};
			//	this.execs.push(func);
			//}
			//else {
			//	func = function (state, Item) {
			//		state.parallel.StartVariable("value");
			//		state.parallel.AppendToVariable("whatever ...");
			//	};
			//	this.execs.push(func);
			//}
			form = "long";
			plural = 0;
			if (this.strings.form) {
				form = this.strings.form;
			}
			if (this.strings.plural) {
				plural = this.strings.plural;
			}
			if ("citation-number" === variable || "year-suffix" === variable || "citation-label" === variable) {
				//
				// citation-number and year-suffix are super special,
				// because they are rangeables, and require a completely
				// different set of formatting parameters on the output
				// queue.
				if (variable === "citation-number") {
					if (state.build.area === "citation") {
						state.opt.update_mode = CSL.NUMERIC;
					}
					if (state.build.area === "bibliography") {
						state.opt.bib_mode = CSL.NUMERIC;
					}
					//this.strings.is_rangeable = true;
					if ("citation-number" === state[state.tmp.area].opt.collapse) {
						this.range_prefix = "-";
					}
					this.successor_prefix = state[state.build.area].opt.layout_delimiter;
					this.splice_prefix = state[state.build.area].opt.layout_delimiter;
					func = function (state, Item, item) {
						id = "" + Item.id;
						if (!state.tmp.just_looking) {
							if (item && item["author-only"]) {
								state.tmp.element_trace.replace("do-not-suppress-me");
								term = CSL.Output.Formatters["capitalize-first"](state, state.getTerm("reference", "long", "singular"));
								state.output.append(term + " ");
								state.tmp.last_element_trace = true;
							}
							if (item && item["suppress-author"]) {
								if (state.tmp.last_element_trace) {
									state.tmp.element_trace.replace("suppress-me");
								}
								state.tmp.last_element_trace = false;
							}
							num = state.registry.registry[id].seq;
							if (state.opt.citation_number_slug) {
								state.output.append(state.opt.citation_number_slug, this);
							} else {
								number = new CSL.NumericBlob(num, this);
								state.output.append(number, "literal");
							}
						}
					};
					this.execs.push(func);
				} else if (variable === "year-suffix") {

					state.opt.has_year_suffix = true;

					if (state[state.tmp.area].opt.collapse === "year-suffix-ranged") {
						this.range_prefix = "-";
					}
					if (state[state.tmp.area].opt["year-suffix-delimiter"]) {
						this.successor_prefix = state[state.build.area].opt["year-suffix-delimiter"];
					}
					func = function (state, Item) {
						if (state.registry.registry[Item.id] && state.registry.registry[Item.id].disambig.year_suffix !== false && !state.tmp.just_looking) {
							//state.output.append(state.registry.registry[Item.id].disambig[2],this);
							num = parseInt(state.registry.registry[Item.id].disambig.year_suffix, 10);
							number = new CSL.NumericBlob(num, this);
							formatter = new CSL.Util.Suffixator(CSL.SUFFIX_CHARS);
							number.setFormatter(formatter);
							state.output.append(number, "literal");
							//
							// don't ask :)
							// obviously the variable naming scheme needs
							// a little touching up
							firstoutput = false;
							len = state.tmp.term_sibling.mystack.length;
							for (pos = 0; pos < len; pos += 1) {
								flag = state.tmp.term_sibling.mystack[pos];
								if (!flag[2] && (flag[1] || (!flag[1] && !flag[0]))) {
									firstoutput = true;
									break;
								}
							}
							// firstoutput = state.tmp.term_sibling.mystack.indexOf(true) === -1;
							specialdelimiter = state[state.tmp.area].opt["year-suffix-delimiter"];
							if (firstoutput && specialdelimiter && !state.tmp.sort_key_flag) {
								state.tmp.splice_delimiter = state[state.tmp.area].opt["year-suffix-delimiter"];
							}
						}
					};
					this.execs.push(func);
				} else if (variable === "citation-label") {
					state.opt.has_year_suffix = true;
					func = function (state, Item) {
						label = Item["citation-label"];
						if (!label) {
							//
							// A shot in the dark
							//
							myname = state.getTerm("reference", "short", 0);
							len = CSL.CREATORS.length;
							for (pos = 0; pos < len; pos += 1) {
								n = CSL.CREATORS[pos];
								if (Item[n]) {
									names = Item[n];
									if (names && names.length) {
										name = names[0];
									}
									if (name && name.family) {
										myname = name.family.replace(/\s+/, "");
									} else if (name && name.literal) {
										myname = name.literal;
										m = myname.toLowerCase().match(/^(a|the|an\s+)/, "");
										if (m) {
											myname = myname.slice(m[1].length);
										}
									}
								}
							}
							year = "0000";
							if (Item.issued) {
								dp = Item.issued["date-parts"];
								if (dp && dp[0] && dp[0][0]) {
									year = "" + dp[0][0];
								}
							}
							label = myname + year;
						}
						suffix = "";
						if (state.registry.registry[Item.id] && state.registry.registry[Item.id].disambig.year_suffix !== false) {
							num = parseInt(state.registry.registry[Item.id].disambig.year_suffix, 10);
							suffix = state.fun.suffixator.format(num);
						}
						label += suffix;
						state.output.append(label, this);
					};
					this.execs.push(func);
				}
			} else {
				if (state.build.term) {
				    term = state.build.term;
				    term = state.getTerm(term, form, plural);
				    if (this.strings["strip-periods"]) {
					term = term.replace(/\./g, "");
				    }
				    // printterm
				    func = function (state, Item) {
					var myterm;
					// if the term is not an empty string, flag this
					// same as a variable with content.
					if (term !== "") {
					    flag = state.tmp.term_sibling.value();
					    flag[0] = true;
					    state.tmp.term_sibling.replace(flag);
					}
					// capitalize the first letter of a term, if it is the
					// first thing rendered in a citation (or if it is
					// being rendered immediately after terminal punctuation,
					// I guess, actually).
					if (!state.tmp.term_predecessor) {
					    //CSL.debug("Capitalize");
					    myterm = CSL.Output.Formatters["capitalize-first"](state, term);
					} else {
					    myterm = term;
					}
					state.output.append(myterm, this);
				    };
				    this.execs.push(func);
				    state.build.term = false;
				    state.build.form = false;
				    state.build.plural = false;
				} else if (this.variables.length) {
					func = function (state, Item) {
						state.parallel.StartVariable(this.variables[0]);
						state.parallel.AppendToVariable(Item[this.variables[0]]);
					};
					this.execs.push(func);

					// plain string fields

					// Deal with multi-fields and ordinary fields separately.
					if (CSL.MULTI_FIELDS.indexOf(this.variables[0]) > -1) {
						// multi-fields
						// Initialize transform factory according to whether
						// abbreviation is desired.
						if (form === "short") {
							// shouldn't third arg be "short"?
							//state.transform.init(this, this.variables[0], this.variables[0]);
							state.transform.init(this, this.variables[0], this.variables[0]);
						} else {
							state.transform.init(this, this.variables[0]);
						}
						if (state.build.area.slice(-5) === "_sort") {
							// multi-fields for sorting get a sort transform,
							// (abbreviated if the short form was selected)
							state.transform.setTransformLocale("locale-sort");
							state.transform.setTransformFallback(true);
							func = state.transform.getOutputFunction();
						} else if (form === "short") {
							 if (["title", "container-title", "collection-title"].indexOf(this.variables[0]) > -1) {
								 // short-form title things get translations maybe
								 state.transform.setTransformLocale("locale-sec");
							 } else {
								 // all other short-form multi-fields for rendering get a locale-pri
								 // transform before abbreviation.
								 state.transform.setTransformLocale("locale-pri");
							 }
							 state.transform.setTransformFallback(true);
							 state.transform.setAbbreviationFallback(true);
							if (this.variables[0] === "container-title") {
								state.transform.setAlternativeVariableName("journalAbbreviation");
							} else if (this.variables[0] === "title") {
								state.transform.setAlternativeVariableName("shortTitle");
							} else if (["publisher", "publisher-place", "edition"].indexOf(this.variables[0]) > -1) {
								// language of publisher and publisher-place follow
								// the locale of the style.
								state.transform.setTransformLocale("default-locale");
							}
							func = state.transform.getOutputFunction();
						} else if (["title", "container-title", "collection-title"].indexOf(this.variables[0]) > -1) {
							// among long-form multi-fields, titles are an
							// exception: they get a locale-sec transform
							// if a value is available.
							state.transform.setTransformLocale("locale-sec");
							state.transform.setTransformFallback(true);
							func = state.transform.getOutputFunction();
						} else {
							// ordinary long-form multi-fields get a locale-pri
							// transform only.
							state.transform.setTransformLocale("locale-pri");
							state.transform.setTransformFallback(true);
							if (["publisher", "publisher-place", "edition"].indexOf(this.variables[0]) > -1) {
								// language of publisher and publisher-place follow
								// the locale of the style.
								state.transform.setTransformLocale("default-locale");
							}
							func = state.transform.getOutputFunction();
						}
						if (this.variables[0] === "container-title") {
							var xfunc = function (state, Item, item) {
								if (Item['container-title'] && state.tmp.citeblob.has_volume) {
									state.tmp.citeblob.can_suppress_identical_year = true;
								}
							};
							this.execs.push(xfunc);
						}
					} else {
						// ordinary fields
						if (CSL.CITE_FIELDS.indexOf(this.variables[0]) > -1) {
							// per-cite fields are read from item, rather than Item
							func = function (state, Item, item) {
								if (item && item[this.variables[0]]) {
									state.output.append(item[this.variables[0]], this);
								}
							};
						} else if (this.variables[0] === "page-first") {
							// page-first is a virtual field, consisting
							// of the front slice of page.
							func = function (state, Item) {
								var idx, value;
								value = state.getVariable(Item, "page", form);
								if (value) {
									idx = value.indexOf("-");
									if (idx > -1) {
										value = value.slice(0, idx);
									}
									state.output.append(value, this);
								}
							};
						} else  if (this.variables[0] === "page") {
							// page gets mangled with the correct collapsing
							// algorithm
							func = function (state, Item) {
								var value = state.getVariable(Item, "page", form);
								if (value) {
									value = state.fun.page_mangler(value);
									state.output.append(value, this);
								}
							};
						} else if ("volume") {
							func = function (state, Item) {
								var value = state.getVariable(Item, this.variables[0], form);
								if (value) {
									// Only allow the suppression of a year identical
									// to the volume number if the container-title
									// is rendered after the volume number.
									state.tmp.citeblob.has_volume = true;
									state.output.append(value, this);
								}
							};
						} else {
							// anything left over just gets output in the normal way.
							func = function (state, Item) {
								var value = state.getVariable(Item, this.variables[0], form);
								if (value) {
									state.output.append(value, this);
								}
							};
						}
					}
					this.execs.push(func);
					func = function (state, Item) {
						state.parallel.CloseVariable("text");
					};
					this.execs.push(func);
				} else if (this.strings.value) {
					// for the text value attribute.
					func = function (state, Item) {
						var flag;
						flag = state.tmp.term_sibling.value();
						flag[0] = true;
						state.tmp.term_sibling.replace(flag);
						state.output.append(this.strings.value, this);
					};
					this.execs.push(func);
					// otherwise no output
				}
			}
			target.push(this);
		}
		CSL.Util.substituteEnd.call(this, state, target);
	}
};


