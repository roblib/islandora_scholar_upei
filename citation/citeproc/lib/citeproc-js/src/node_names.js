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

CSL.Node.names = {
	build: function (state, target) {
		var debug, func, len, pos, attrname;
		debug = false;

		if (this.tokentype === CSL.START || this.tokentype === CSL.SINGLETON) {
			CSL.Util.substituteStart.call(this, state, target);
			state.build.substitute_level.push(1);

			state.fixOpt(this, "names-delimiter", "delimiter");

			// init names
			func = function (state, Item, item) {
				var namesets, nameset, names, rawlist, after_people_set, pers_seen, in_orgs, last_type, name, len, pos, variable, rawvar, llen, ppos, lllen, pppos, lllst, end, mynameset, tnamesets, frontnames, pair, offset, swaplist;
				state.parallel.StartVariable("names");
				if (state.tmp.value.length === 0) {
					namesets = [];
					len = this.variables.length;
					if (len && state.opt.xclass === "in-text") {
						len = 1;
					}
					for (pos = 0; pos < len; pos += 1) {
						variable = this.variables[pos];
						//SNIP-START
						if (debug) {
							CSL.debug(">>>> variable: " + variable);
						}
						//SNIP-END
						if (Item[variable]) {
							rawlist = Item[variable];
							if ("string" === typeof Item[variable]) {
								rawlist = [{literal: Item[variable]}];
							}
							var rawlen = rawlist.length;
							if (state.opt.max_number_of_names && rawlen > 50 && rawlen > (state.opt.max_number_of_names + 2)) {
								rawlist = rawlist.slice(0, state.opt.max_number_of_names + 2);
							}
							
							names = [];
							//
							// we start with this:
							//
							//  {pers}, {orgs1}, {pers1a}, {pers1b}, {orgs2} ...
							//
							// where any {pers*} may be missing.
							//
							// We want a list of namesets with markers.
							// Namesets are composed of only pers or only
							// org.
							//
							// Markers:
							//
							//   variable (change triggers label and join)
							//   species:pers (triggers et-al join and external rendering)
							//   after_people (set once on 1st org, triggers "with" joiner)
							//   pers_org_start (sets pers-org join)
							//   species:org (triggers external rendering)
							//   pers_org_end (finalizes pers-org join)
							//   organization_start (first person or org of an organization)
							//   organization_end  (totally last organiztion)
							//
							// state vars
							tnamesets = [];
							nameset = {names: []};
							frontnames = [];
							llen = rawlist.length;
							for (ppos = 0; ppos < llen; ppos += 1) {
								name = rawlist[ppos];
								//
								// Force all institutional names to literal.
								//
								if (name.literal || (name.family && !name.given)) {
									//print("NAME LITERAL");
									// org
									nameset.variable = variable;
									nameset.species = "org";
									if (name.literal) {
										var lit = name.literal;
									} else {
										var lit = name.family;
									}
									if (lit.slice(0, 1) === '"' && lit.slice(-1)) {
										lllst = [lit.slice(1, -1)];
									} else {
										lllst = lit.split(/,\s+/);
									}
									lllen = lllst.length;
									for (pppos = 0; pppos < lllen; pppos += 1) {
										name = {literal: lllst[pppos], family:'', given:''};
										nameset.names.push(name);
									}
									tnamesets.push(nameset);
									nameset = {names: []};
								} else {
									//print("NAME PERSONAL");
									// pers
									nameset.variable = variable;
									nameset.species = "pers";
									nameset.names.push(name);
									//print("Whoa: "+rawlist.length+" "+rawlist[ppos + 1].family);
									if (rawlist.length === (ppos + 1) || rawlist[ppos + 1].literal || (rawlist[ppos + 1].family && !rawlist[ppos + 1].given)) {
										tnamesets.push(nameset);
										nameset = {names: []};
									}
								}
								//   * variable (change triggers label and join)
								//   * species:pers (triggers et-al join and external rendering)
								//   after_people (set once on 1st org, triggers "with" joiner)
								//   * organization_first (start of names with orgs)
								//   * species:org (triggers external rendering)
								//   * organization_last (end of names with orgs)
							}

							//print("type: "+tnamesets[0].species);
							if (tnamesets.length > 1 && tnamesets.slice(-1)[0].species === "pers") {
								frontnames = tnamesets.slice(-1);
								tnamesets = tnamesets.slice(0, tnamesets.length - 1);
								if (tnamesets.length > 0) {
									tnamesets[0].after_people = true;
								}
							}  else {
								frontnames = [];
							}
							if (tnamesets.length > 0 && tnamesets.slice(-1)[0].species === "org") {
								tnamesets[0].organization_first = true;
								tnamesets.slice(-1)[0].organization_last = true;
								if (frontnames.length) {
									// the free agent wrapper can be used to
									// lift out trailing names as well as  for
									// the "with" join
									frontnames[0].free_agent_start = true;
									tnamesets.slice(-1)[0].free_agent_end = true;
								}
							}
							if (frontnames.length === 0) {
								if (tnamesets.length > 1) {
									if (tnamesets[0].species === "pers") {
										//pers
										// inside inner
										tnamesets[1].trailers1_start = true;
										if (tnamesets.length === 2) {
											// close at the end
											tnamesets[1].trailers1a_end = true;
										} else {
											// close at the flip
											tnamesets[1].trailers1b_end = true;
										}
										if (tnamesets.length > 2) {
											// outside inner, at pos 2 and at end
											tnamesets[2].trailers2_start = true;
											tnamesets.slice(-1)[0].trailers2_end = true;
										}
									} else {
										//org
										// outside inner, pos 1 and at end
										tnamesets[1].trailers2_start = true;
										tnamesets.slice(-1)[0].trailers2_end = true;
									}
								}
							} else {
								// outside with-group
								tnamesets[0].trailers3_start = true;
								tnamesets.slice(-1)[0].trailers3_end = true;
							}
							tnamesets = frontnames.concat(tnamesets);
							namesets = namesets.concat(tnamesets);
						}
					}
					if ((state.opt.xclass === "in-text" && state.tmp.area.slice(0, 8) === "citation") || (state.opt.xclass === "note" && item && "number" === typeof item.position && item.position !== CSL.POSITION_FIRST)) {
//					if ((state.opt.xclass === "in-text" && state.tmp.area.slice(0, 8) === "citation")) {

						//print("Hi: "+namesets.length);
						if (namesets.length > 1) {
							// Similar logic to that embedded in evaluateLabel
							// (see node_label.js)
							// The two functions should probably be combined, but
							// not possible currently, where they work
							// on different structures of the same data.
							// One for later.
							var creatorCount = -1;
							var lastWasPerson = true;
							state.tmp.forceEtAl = false;
							for (var i = 0, ilen = namesets.length; i < ilen; i += 1) {
								if ("pers" === namesets[i].species) {
									creatorCount += 1;
									lastWasPerson = true;
								} else {
									if (!lastWasPerson) {
										creatorCount += 1;
									}
									lastWasPerson = false;
								}
								if (creatorCount) {
									state.tmp.forceEtAl = true;
									break;
								}
							}
						}
						namesets = namesets.slice(0, 1);
						if (namesets.length) {
							if (namesets[0].species === "pers") {
								namesets[0].organization_first = false;
								namesets[0].after_people = false;
								namesets[0].free_agent_start = false;
								namesets[0].free_agent_end = false;
							} else {
								namesets[0].organization_last = true;
							}
						}
					}
					for (pos = 0, len = namesets.length; pos < len; pos += 1) {
						state.tmp.names_max.push(namesets[pos].names.length);
					}

					state.tmp.value = namesets.slice();

					//print("CHECK: "+namesets[0].names.length);

					//SNIP-START
					if (debug) {
						len = namesets.length;
						for (pos = 0; pos < len; pos += 1) {
							mynameset = namesets[pos];
							CSL.debug(mynameset.species);
						}
					}
					//SNIP-END
				}
			};
			this.execs.push(func);
		}

		if (this.tokentype === CSL.START) {

			state.build.names_flag = true;

			// init can substitute
			func = function (state, Item) {
				state.tmp.can_substitute.push(true);
			};
			this.execs.push(func);

			// init names
			func = function (state, Item) {
				state.output.startTag("names", this);
				//SNIP-START
				if (debug) {
					CSL.debug("## startTag: names " + this.strings.suffix);
				}
				//SNIP-END
				state.tmp.name_node = state.output.current.value();
			};
			this.execs.push(func);

		}

		if (this.tokentype === CSL.END) {

			len = CSL.NAME_ATTRIBUTES.length;
			for (pos = 0; pos < len; pos += 1) {
				attrname = CSL.NAME_ATTRIBUTES[pos];
				if (attrname.slice(0, 5) === "et-al") {
					continue;
				}
				if ("undefined" !== typeof state.build.nameattrs[attrname]) {
					this.strings[attrname] = state.build.nameattrs[attrname];
					delete state.build.nameattrs[attrname];
				}
			}

			// handle names
			func = function (state, Item, item) {
				var common_term, nameset, name, local_count, withtoken, namesetIndex, lastones, currentones, compset, display_names, suppress_min, suppress_condition, sane, discretionary_names_length, overlength, et_al, and_term, outer_and_term, use_first, append_last, delim, param, paramx, val, s, myform, myinitials, termname, form, namepart, namesets, llen, ppos, label, plural, last_variable, cutinfo, obj, et_al_pers, et_al_org, and_pers, and_org, with_term, chk, apply_ellipsis;
				namesets = [];
				common_term = CSL.Util.Names.getCommonTerm(state, state.tmp.value);
				if (common_term) {
					namesets = state.tmp.value.slice(0, 1);
				} else {
					namesets = state.tmp.value;
				}
				//
				// Normalize names for which it is requested
				//
				len = namesets.length;
				//SNIP-START
				if (debug) {
					CSL.debug("namesets.length[1]: " + namesets.length);
				}
				//SNIP-END



				// chop names in first nameset by value in names_slice,
				// suppress the initially listed author in
				// subsequent renderings within a cite.  This works a little
				// differently for personal and institutional authors.
				if (namesets.length && (state.tmp.area === "bibliography" || state.tmp.area === "bibliography_sort" || (state.tmp.area && state.opt.xclass === "note"))) {
				    // save off the varname for safekeeping, for use
				    // in optional trimming of names. This stuff
				    // around cut_var is special pleading for
				    // citeproc-js support of author listings,
				    // quite messy and arcane.
				    if (state.tmp["et-al-min"] === 1 && state.tmp["et-al-use-first"] === 1) {
					state.tmp.cut_var = namesets[0].variable;
				    }
				    cutinfo = state.tmp.names_cut;
				    // (seems to be where aggressive suppression happens)
				    if (namesets[0].species === "pers") {
					if (state.tmp.cut_var) {
					    namesets[0].names = namesets[0].names.slice(cutinfo.counts[state.tmp.cut_var]);
					}
					
					if (namesets[0].names.length === 0) {
					    if (namesets[0].free_agent_start) {
						namesets[1].free_agent_start = true;
					    }
					    if (namesets[0].organization_first) {
						namesets[1].organization_first = true;
					    }
					    namesets = namesets.slice(1);
					}
				    } else {
					namesets = namesets.slice(0, 1);
					if (namesets[0].organization_first) {
					    namesets[0].organization_last = true;
					}
				    }
				    // should always be true, but just in case
				    // this slices off subsequent namesets in the initial name
				    // rendered, when the same name is rendered a second time.
				    // Useful for robust per-author listings.
				    if (state.tmp.cut_var && cutinfo.used === state.tmp.cut_var) {
					llen = cutinfo.variable[state.tmp.cut_var].length - 1;
					for (ppos = llen; ppos > -1; ppos += -1) {
					    obj = cutinfo.variable[state.tmp.cut_var][ppos];
					    obj[0].blobs = obj[0].blobs.slice(0, obj[1]).concat(obj[0].blobs.slice(obj[1] + 1));
					}
				    }
				}
				if (!state.output.getToken("institution")) {
					state.output.addToken("institution");
				}
				len = namesets.length;
				for (pos = 0; pos < len; pos += 1) {
					nameset = namesets[pos];
					if ("org" === nameset.species) {
						if (state.output.getToken("institution").strings["reverse-order"]) {
							nameset.names.reverse();
						}
					}
				}
				local_count = 0;
				nameset = {};

				// Various formatting tokens needed for names output.
				state.output.addToken("term-join");
				state.output.addToken("etal-join");

				state.output.addToken("space", " ");
				state.output.addToken("sortsep", state.output.getToken("name").strings["sort-separator"]);
				state.output.addToken("suffixsep", " ");

				// Style token for et-al element with personal names.
				// The prefix of this element will be overwritten by the -single
				// and -multiple variants on the fly.
				if (!state.output.getToken("et-al-pers")) {
					state.output.addToken("et-al-pers");
				}
				var nametok = state.output.getToken("name");
				if (nametok.strings["delimiter-precedes-et-al"] === "always") {
					state.output.getToken("et-al-pers").strings["prefix-single"] = nametok.strings.delimiter;
					state.output.getToken("et-al-pers").strings["prefix-multiple"] = nametok.strings.delimiter;
				} else if (nametok.strings["delimiter-precedes-et-al"] === "never") {
					state.output.getToken("et-al-pers").strings["prefix-single"] = " ";
					state.output.getToken("et-al-pers").strings["prefix-multiple"] = " ";
				} else {
					state.output.getToken("et-al-pers").strings["prefix-single"] = " ";
					state.output.getToken("et-al-pers").strings["prefix-multiple"] = nametok.strings.delimiter;
				}
				et_al_pers = state.getTerm("et-al", "long", 0);
				if ("undefined" !== typeof state.output.getToken("et-al-pers").strings.term) {
					et_al_pers = state.output.getToken("et-al-pers").strings.term;
				}

				// Style token for et-al element with institutional names.
				// The prefix of this element will be overwritten by the -single
				// and -multiple variants on the fly.
				// XXX: Not yet hooked up, institutional et al. untested
				// XXX: needs same handling of delimiter-precedes-et-al as above?
				if (!state.output.getToken("et-al-org")) {
					state.output.addToken("et-al-org");
				}
				state.output.getToken("et-al-org").strings["prefix-single"] = " ";
				state.output.getToken("et-al-org").strings["prefix-multiple"] = ", ";
				et_al_org = state.getTerm("et-al", "long", 0);

				// Style token for and element with personal names.
				// The prefix of this element will be overwritten by the -single
				// and -multiple variants on the fly, if the and attribute
				// is set on cs:name.  Otherwise prefix is set to nil, and
				// and_pers is set to the value of the cs:name delimiter
				// attribute.
				// XXX: Not yet hooked up
				if (!state.output.getToken("and-pers")) {
					state.output.addToken("and-pers");
					// Conditional goes here
				}
				state.output.getToken("and-pers").strings["prefix-single"] = " ";
				state.output.getToken("and-pers").strings["prefix-multiple"] = ", ";
				// Conditional goes here
				and_pers = state.getTerm("and", "long", 0);

				// Style token for and element with instiutional names.
				// The prefix of this element will be overwritten by the -single
				// and -multiple variants on the fly, if the and attribute
				// is set on cs:name.  Otherwise prefix is set to nil, and
				// and_org is set to the value of the cs:institution delimiter
				// attribute.
				// XXX: Not yet hooked up
				if (!state.output.getToken("and-org")) {
					state.output.addToken("and-org");
					// Conditional goes here
				}
				state.output.getToken("and-org").strings["prefix-single"] = " ";
				state.output.getToken("and-org").strings["prefix-multiple"] = ", ";
				// Conditional goes here
				and_org = state.getTerm("and", "long", 0);

				state.output.addToken("with");
				state.output.getToken("with").strings.prefix = ", ";
				state.output.getToken("with").strings.suffix = " ";
				with_term = "with";

				// not yet hooked up; using direct output instead.
				// The term and the join need to be separated here,
				// so that subsequent renderings of the name will
				// be handled gracefully when the "with" join
				// comes in front.

				//state.output.addToken("with-join");
				//state.output.getToken("with-join").strings.delimiter = ", ";

				//state.output.addToken("with-group");
				//state.output.getToken("with-group").strings.delimiter = " ";

				state.output.addToken("trailing-names");

				outer_and_term = " " + state.output.getToken("name").strings.and + " ";
				state.output.addToken("institution-outer", outer_and_term);

				//if ("undefined" === typeof state.output.getToken("etal").strings.et_al_term) {
				//	state.output.getToken("etal").strings.et_al_term = state.getTerm("et-al", "long", 0);
				//}

				if (!state.output.getToken("label")) {
					state.output.addToken("label");
				}

				delim = state.output.getToken("name").strings.delimiter;
				state.output.addToken("inner", delim);

				//
				// Locale term not yet available. (this approach didn't make sense
				// anyway.  with is handled further down below.)
				//
				// if ("undefined" === typeof state.output.getToken("with").strings.with_term){
				// state.output.getToken("with").strings.with_term = state.getTerm("with","long",0);
				// }
				//if ("undefined" === typeof state.output.getToken("with").strings.with_term){
				//	state.output.getToken("with").strings.with_term = "with";
				//}
				state.output.addToken("commasep", ", ");

				len = CSL.DECORABLE_NAME_PARTS.length;
				for (pos = 0; pos < len; pos += 1) {
					namepart = CSL.DECORABLE_NAME_PARTS[pos];
					if (!state.output.getToken(namepart)) {
						state.output.addToken(namepart);
					}
				}
				state.output.addToken("dropping-particle", false, state.output.getToken("family"));
				state.output.addToken("non-dropping-particle", false, state.output.getToken("family"));
				state.output.addToken("suffix", false, state.output.getToken("family"));
				state.output.getToken("suffix").decorations = [];

				// open for term join, for any and all names.
				state.output.openLevel("term-join");

				len = namesets.length;
				//SNIP-START
				if (debug) {
					CSL.debug("namesets.length[2]: " + namesets.length);
				}
				//SNIP-END
				for  (namesetIndex = 0; namesetIndex < len; namesetIndex += 1) {

					nameset = namesets[namesetIndex];
					//
					// configure label if poss
					label = false;
					var labelnode = state.output.getToken("label");
					if (state.output.getToken("label").strings.label_position) {
						if (common_term) {
							termname = common_term;
						} else {
							termname = nameset.variable;
						}
						label = CSL.evaluateLabel(labelnode, state, Item, item, termname, nameset.variable);
					}

					if (label && state.output.getToken("label").strings.label_position === CSL.BEFORE) {
						state.output.append(label, "label");
					}

					if (!state.tmp.disambig_request) {
						state.tmp.disambig_settings.givens[state.tmp.nameset_counter] = [];
					}

					display_names = nameset.names.slice();

					if ("pers" === nameset.species) {
						//
						// the names constraint (experimental)
						//
						suppress_min = state.output.getToken("name").strings["suppress-min"];

						// set the number of names to be _intended_ for rendering,
						// in the first nameset, if personal, for subsequent slicing.
						if (namesetIndex === 0 && !suppress_min && (state.tmp.area === "bibliography" || (state.tmp.area === "citation" && state.opt.xclass === "note"))) {
							state.tmp.names_cut.counts[nameset.variable] = state.tmp["et-al-use-first"];
						}

						sane = state.tmp["et-al-min"] >= state.tmp["et-al-use-first"];
						if (state.tmp["et-al-use-last"] && state.tmp["et-al-min"] >= state.tmp["et-al-use-first"] + 2) {
							apply_ellipsis = true;
						} else {
							apply_ellipsis = false;
						}
						//
						// if there is anything on name request, we assume that
						// it was configured correctly via state.names_request
						// by the function calling the renderer.
						discretionary_names_length = state.tmp["et-al-min"];

						//
						// Invoke names constraint
						//
						suppress_condition = suppress_min && display_names.length >= suppress_min;
						if (suppress_condition) {
							continue;
						}

						//
						// if rendering for display, do not honor a disambig_request
						// to set names length below et-al-use-first, and do not
						// truncate unless number of names is equal to or greater
						// than et-al-min
						//
						if (state.tmp.suppress_decorations) {
							if (state.tmp.disambig_request) {
								discretionary_names_length = state.tmp.disambig_request.names[state.tmp.nameset_counter];
							} else if (display_names.length >= state.tmp["et-al-min"]) {
								discretionary_names_length = state.tmp["et-al-use-first"];
							}
						} else {
							if (state.tmp.disambig_request && state.tmp.disambig_request.names[state.tmp.nameset_counter] > state.tmp["et-al-use-first"]) {
								if (display_names.length < state.tmp["et-al-min"]) {
									discretionary_names_length = display_names.length;
								} else {
									discretionary_names_length = state.tmp.disambig_request.names[state.tmp.nameset_counter];
								}
							} else if (display_names.length >= state.tmp["et-al-min"]) {
								discretionary_names_length = state.tmp["et-al-use-first"];
							}
						}
						overlength = display_names.length > discretionary_names_length;
						// This var is used to control contextual join, and
						// lies about the number of names when forceEtAl is true,
						// unless normalized.
						if (discretionary_names_length > display_names.length) {
							discretionary_names_length = display_names.length;
						}
						et_al = false;
						and_term = "";
						// forceEtAl is relevant when the author list is
						// truncated to eliminate clutter.
						if (sane && (overlength || state.tmp.forceEtAl)) {
							if (! state.tmp.sort_key_flag) {
								et_al = et_al_pers;
								//et_al = state.output.getToken("etal").strings.et_al_term;

								// XXXXX: temporary hack to exhibit existing context-sensitive
								// et al. join behavior.
								if (discretionary_names_length > 1) {
									state.output.getToken("et-al-pers").strings.prefix = state.output.getToken("et-al-pers").strings["prefix-multiple"];
								} else {
									state.output.getToken("et-al-pers").strings.prefix = state.output.getToken("et-al-pers").strings["prefix-single"];
								}
							}
							if (apply_ellipsis) {
								state.tmp.use_ellipsis = true;
								display_names = display_names.slice(0, discretionary_names_length).concat(display_names.slice(-1));
							} else {
								display_names = display_names.slice(0, discretionary_names_length);
							}
						} else {
							if (!state.tmp.sort_key_flag) {
								if (display_names.length > 1) {
									if (state.output.getToken("name").strings.and) {
										and_term = state.output.getToken("name").strings.and;
									}
								}
							}
						}
						state.output.formats.value().name.strings.delimiter = and_term;
					} else {
						// org
						// set the number of names to be _intended_ for rendering,
						// in the first nameset, if personal, for subsequent slicing.
						if (namesetIndex === 0 && (state.tmp.area === "bibliography" || (state.tmp.area === "citation" && state.opt.xclass === "note"))) {
							state.tmp.names_cut.counts[nameset.variable] = 1;
						}
						use_first = state.output.getToken("institution").strings["use-first"];
						if (!use_first && namesetIndex === 0) {
							use_first = state.output.getToken("institution").strings["substitute-use-first"];
						}
						if (!use_first) {
							use_first = 0;
						}
						append_last = state.output.getToken("institution").strings["use-last"];
						if (use_first || append_last) {
							s = display_names.slice();
							display_names = [];
							display_names = s.slice(0, use_first);
							s = s.slice(use_first);
							if (append_last) {
								if (append_last > s.length) {
									append_last = s.length;
								}
								if (append_last) {
									display_names = display_names.concat(s.slice((s.length - append_last)));
								}
							}
						}
					}
					state.tmp.disambig_settings.names[state.tmp.nameset_counter] = display_names.length;
					local_count += display_names.length;

					state.tmp.names_used.push({names:display_names,etal:et_al});

					if (!state.tmp.suppress_decorations && (state[state.tmp.area].opt.collapse === "year" || state[state.tmp.area].opt.collapse === "year-suffix" || state[state.tmp.area].opt.collapse === "year-suffix-ranged")) {
						//
						// This is fine, but the naming of the comparison
						// function is confusing.  This is just checking whether the
						// current name is the same as the last name rendered
						// in the last cite, and it works.  Set a toggle if the
						// test fails, so we can avoid further suppression in the
						// cite.
						//
						if (state.tmp.last_names_used.length === state.tmp.names_used.length) {
							// lastones = state.tmp.last_names_used[state.tmp.nameset_counter];
							lastones = state.tmp.last_names_used[state.tmp.nameset_counter];
							//lastones = state.tmp.last_names_used;
							currentones = state.tmp.names_used[state.tmp.nameset_counter];
							//currentones = state.tmp.names_used;
							compset = [currentones, lastones];
							if (CSL.Util.Names.compareNamesets(lastones,currentones)) {
								continue;
							} else {
								state.tmp.have_collapsed = false;
							}
						}
					}

					//
					// "name" is the format for the outermost nesting of a nameset
					// "inner" is a format consisting only of a delimiter, used for
					// joining all but the last name in the set together.

					//SNIP-START
					if (debug) {
						CSL.debug("nameset.names.length[1]: " + nameset.names.length);
					}
					//SNIP-END
					// DON'T DO THIS IF NO NAMES IN SUBSEQUENT FORM
					llen = nameset.names.length;
					for (ppos = 0; ppos < llen; ppos += 1) {
						//
						// register the name in the global names disambiguation
						// registry
						state.registry.namereg.addname("" + Item.id, nameset.names[ppos], ppos);
						chk = state.tmp.disambig_settings.givens[state.tmp.nameset_counter];
						if ("undefined" === typeof chk) {
							state.tmp.disambig_settings.givens.push([]);
						}
						chk = state.tmp.disambig_settings.givens[state.tmp.nameset_counter][ppos];
						if ("undefined" === typeof chk) {
							myform = state.output.getToken("name").strings.form;
							myinitials = this.strings["initialize-with"];
							param = state.registry.namereg.evalname("" + Item.id, nameset.names[ppos], ppos, 0, myform, myinitials);
							state.tmp.disambig_settings.givens[state.tmp.nameset_counter].push(param);
						}
						//
						// set the display mode default for givennames if required
						myform = state.output.getToken("name").strings.form;
						myinitials = this.strings["initialize-with"];
						paramx = state.registry.namereg.evalname("" + Item.id, nameset.names[ppos], ppos, 0, myform, myinitials);
						if (state.tmp.sort_key_flag) {
							state.tmp.disambig_settings.givens[state.tmp.nameset_counter][ppos] = 2;
							param = 2;
						} else if (state.tmp.disambig_request) {
							//
							// fix a request for initials that makes no sense.
							// can't do this in disambig, because the availability
							// of initials is not a global parameter.
							val = state.tmp.disambig_settings.givens[state.tmp.nameset_counter][ppos];
							// This is limited to by-cite disambiguation.
							if (val === 1 && 
								state.opt["givenname-disambiguation-rule"] === "by-cite" && 
								"undefined" === typeof this.strings["initialize-with"]) {
								val = 2;
							}
							param = val;
//							if (state[state.tmp.area].opt["disambiguate-add-givenname"] && state[state.tmp.area].opt["givenname-disambiguation-rule"] != "by-cite"){
							if (state.opt["disambiguate-add-givenname"]) {
								param = state.registry.namereg.evalname("" + Item.id, nameset.names[ppos], ppos, param, state.output.getToken("name").strings.form, this.strings["initialize-with"]);
							}
						} else {
							//
							// it clicks.  here is where we will put the
							// call to the names register, to get the floor value
							// for an individual name.
							//
							param = paramx;
						}
						// Need to save off the settings based on subsequent
						// form, when first cites are rendered.  Otherwise you
						// get full form names everywhere.
						if (!state.tmp.just_looking && item && item.position === CSL.POSITION_FIRST) {
							param = paramx;
						}
						if (!state.tmp.sort_key_flag) {
							state.tmp.disambig_settings.givens[state.tmp.nameset_counter][ppos] = param;
						}
					}
					//
					// Nesting levels are opened to control joins with
					// content at the end of the names block
					//

					// markers are:
					//
					//   variable (change triggers label and join)
					//   species:pers (triggers et-al join and external rendering)
					//   after_people (set once on 1st org, triggers "with" joiner)
					//   pers_org_start (sets pers-org join)
					//   species:org (triggers external rendering)
					//   pers_org_end (finalizes pers-org join)
					//


					if (namesetIndex > 0 && nameset.variable !== last_variable) {
						//SNIP-START
						if (debug) {
							CSL.debug("-- blink 'term-join'");
						}
						//SNIP-END
						state.output.closeLevel("term-join");
					}
					if (namesetIndex > 0 && nameset.variable !== last_variable) {
						state.output.openLevel("term-join");
					}

					//if (nameset.free_agent_start) {
					//	state.output.openLevel("with-join");
					//}

					if (nameset.trailers3_start) {
						state.output.openLevel("trailing-names", state.tmp.cut_var);
					}
					if (nameset.after_people) {
						//SNIP-START
						if (debug) {
							CSL.debug("-- reached 'after_people'");
						}
						//SNIP-END
						//state.output.openLevel("with-group");
						state.output.append("with", "with");
					}

					if (nameset.organization_first) {
						//SNIP-START
						if (debug) {
							CSL.debug("-- reached 'organization_first'");
						}
						//SNIP-END
						state.output.openLevel("institution-outer");
					}
					if (nameset.trailers2_start) {
						state.output.openLevel("trailing-names", state.tmp.cut_var);
					}
					if (nameset.organization_first) {
						state.output.openLevel("inner");
					}
					if (nameset.trailers1_start) {
						state.output.openLevel("trailing-names", state.tmp.cut_var);
					}

					//if (nameset.pers_org_start) {
					//	if (debug) {
					//		CSL.debug("-- reached 'pers_org_start'");
					//	}
					//	//state.output.openLevel("inner");
					//}


					if (nameset.species === "pers") {
						// pers
						//SNIP-START
						if (debug) {
							CSL.debug("-- reached species 'pers'");
						}
						//SNIP-END
						state.output.openLevel("etal-join"); // join for etal
						CSL.Util.Names.outputNames(state, display_names);
						if (et_al && !state.tmp.use_ellipsis) {
							state.output.append(et_al, "et-al-pers");
						}
						state.output.closeLevel("etal-join"); // etal
					} else {
						//org
						CSL.Util.Institutions.outputInstitutions(state, display_names);
						if (nameset.organization_last) {
							//SNIP-START
							if (debug) {
								CSL.debug("-- reached 'organization_last'");
							}
							//SNIP-END
							if (nameset.trailers1a_end) {
								state.output.closeLevel("trailing-names");
							}
							state.output.closeLevel("inner");
							if (nameset.trailers2_end) {
								state.output.closeLevel("trailing-names");
							}
							state.output.closeLevel("institution-outer");
						} else {
							//SNIP-START
							if (debug) {
								CSL.debug("-- reached 'organization_NOT_last'");
							}
							//SNIP-END
							if (nameset.trailers1b_end) {
								state.output.closeLevel("trailing-names");
							}
							state.output.closeLevel("inner");
							state.output.openLevel("inner");
						}
					}


					//if (nameset.pers_org_end) {
					//	if (debug) {
					//		CSL.debug("-- reached 'pers_org_end'");
					//	}
					//	//state.output.closeLevel("inner");
					//}

					//if (nameset.free_agent_end) {
					//	state.output.closeLevel("with-group");
					//}
					if (nameset.trailers3_end) {
						state.output.closeLevel("trailing-names");
					}
					//if (nameset.free_agent_end) {
					//	state.output.closeLevel("with-join");
					//}

					// lookahead
					if (namesets.length === namesetIndex + 1 || namesets[namesetIndex + 1].variable !== namesets[namesetIndex].variable) {
						if (label && state.output.getToken("label").strings.label_position !== CSL.BEFORE) {
							state.output.append(label, "label");
						}
					}

					state.tmp.nameset_counter += 1;
					last_variable = nameset.variable;
				}

				state.output.closeLevel("term-join");

				if (state.output.getToken("name").strings.form === "count") {
					state.output.clearlevel();
					state.output.append(local_count.toString());
					state.tmp["et-al-min"] = false;
					state.tmp["et-al-use-first"] = false;
				}
			};
			// handle names
			this.execs.push(func);
		}

		if (this.tokentype === CSL.END) {
			// unsets
			func = function (state, Item) {
				if (!state.tmp.can_substitute.pop()) {
					state.tmp.can_substitute.replace(false, CSL.LITERAL);
				}
				CSL.Util.Names.reinit(state, Item);
				// names
				//SNIP-START
				if (debug) {
					CSL.debug("## endTag: names");
				}
				//SNIP-END
				state.output.endTag();

				state.parallel.CloseVariable("names");

				state.tmp["has-institution"] = false;
				state.tmp["has-first-person"] = false;

				state.tmp["et-al-min"] = false;
				state.tmp["et-al-use-first"] = false;
				state.tmp["et-al-use-last"] = false;
				state.tmp.use_ellipsis = false;

				state.tmp.can_block_substitute = false;
				
				state.tmp.forceEtAl = false;

			};
			this.execs.push(func);

			state.build.names_flag = false;
			state.build.name_flag = false;

		}
		target.push(this);

		if (this.tokentype === CSL.END || this.tokentype === CSL.SINGLETON) {
			state.build.substitute_level.pop();
			CSL.Util.substituteEnd.call(this, state, target);
		}
	},

	//
	// XXXXX: in configure phase, set a flag if this node contains an
	// institution node.  If it does, then each nameset will be filtered into an
	 // array containing two lists, to be run separately and joined
	// in the end.  If we don't, the array will contain only one list.
	//
	configure: function (state, pos) {
		if ([CSL.SINGLETON, CSL.START].indexOf(this.tokentype) > -1) {
			if (state.build.has_institution) {
				this.strings["has-institution"] = true;
				state.build.has_institution = false;
			}
		}
	}
};
