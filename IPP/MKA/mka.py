#!/usr/bin/env python3
# -*- coding: UTF-8 -*-
#MKA:xsevci50

import sys
import re
    
class Symbol(object):
    def __init__(self, name):
        self.name = name
        
    def __cmp__(self, other):
        if self.name < other.name:
            return -1
        elif self.name == other.name:
            return 0
        else:
            return 1
        
    def __eq__(self, other):
        if type(other) is str:
            return self.name == other
        else:
            return self is other
        
    def __lt__(self, other):
        return self.name < other.name
        
    def __str__(self):
        tmp = str(self.name)
        return "'" + tmp.replace("'", "''") + "'"
            
class Rule(object):
    def __init__(self, stateFrom, transSymbol, stateTo):
        self.stateFrom = stateFrom
        self.symbol = transSymbol
        self.stateTo = stateTo
        
    def __cmp__(self, other):
        if self.stateFrom < other.stateFrom:
            return -1
        elif self.stateFrom == other.stateFrom:
            if self.symbol < other.symbol:
                return -1
            elif self.symbol == other.symbol:
                if self.stateTo < other.stateTo:
                    return -1
                elif self.stateTo == other.stateTo:
                    return 0
                else:
                    return 1
            else:
                return 1
        else:
            return 1
        
    def __eq__(self, other):
        if type(other) is Rule:
            return self.stateFrom == other.stateFrom and \
            self.symbol == other.symbol and \
            self.stateTo == other.stateTo
        else:
            return False
        
    def __lt__(self, other):
        if self.stateFrom == other.stateFrom:
            if self.symbol == other.symbol:
                return self.stateTo < other.stateTo
            else:
                return self.symbol < other.symbol
        else:
            return self.stateFrom < other.stateFrom
        
    def __str__(self):
        return str(self.stateFrom) + " " + str(self.symbol) + " -> " + str(self.stateTo)
        
class State(object):
    def __init__(self, name):
        self.name = name
        
    def __cmp__(self, other):
        if self.name < other.name:
            return -1
        elif self.name == other.name:
            return 0
        else:
            return 1
        
    def __eq__(self, other):
        if type(other) is str:
            return self.name == other
        else:
            return self is other
        
    def __lt__(self, other):
        return self.name < other.name
        
    def __str__(self):
        return self.name

class Settings(object):
    def __init__(self):
        self.inFile = sys.stdin
        self.outFile = sys.stdout
        self.help = False
        self.findNonFinishing = False
        self.minimize = False
        self.caseIns = False
        self.wsfa = False
        
        
def printError(message):
    sys.stderr.write("Error: " + message + "\n")

class StateGroup(object):
    def __init__(self, states):
        self.states = []
        # Create copy of list
        for state in states:
            self.states.append(state)
            
    def __eq__(self, other):
        return self is other
            
    def __len__(self):
        return len(self.states)
    
    def __iter__(self):
        for state in self.states:
            yield state
            
    def __hash__(self):
        return hash(repr(self))
    
    def __str__(self):
        string = ""
        for state in self.states[0:-1]:
            string += str(state) + "_"
        string += str(self.states[-1])
        return string
    
class FA(object):
    def __init__(self, components):
        self.alphabet = []
        self.states = []
        self.rules = []
        self.start = None
        self.final = []
        
        for i, component in enumerate(components):
            if i == 0:
                for state in component:
                    if state.content not in self.states:
                        self.states.append(State(state.content))
            elif i == 1:
                for symbol in component:
                    if symbol.content not in self.alphabet:
                        self.alphabet.append(Symbol(symbol.content))
            elif i == 2:
                for j in range(0, int(len(component) / 3)):
                    try:
                        stateFrom = self.states[self.states.index(component[j*3].content)]
                    except:
                        printError("The 'stateFrom': " + component[j*3].content + " used in rule was not declared as state.")
                        raise
                    try:
                        symbol = self.alphabet[self.alphabet.index(component[j*3 + 1].content)]
                    except:
                        printError("The symbol: " + component[j*3 + 1].content + " used in rule was not declared as symbol.")
                        raise
                    try:
                        stateTo = self.states[self.states.index(component[j*3 + 2].content)]
                    except:
                        printError("The 'stateTo': " + component[j*3 + 2].content + " used in rule was not declared as state.")
                        raise
                    rule = Rule(stateFrom, symbol, stateTo)
                    if rule not in self.rules:
                        self.rules.append(rule)
            elif i == 3:
                try:
                    self.start = self.states[self.states.index(component[0].content)]
                except:
                    printError("The starting state: " + component[0].content + " was not declared as state.")
                    raise
            elif i == 4:
                for finalState in component:
                    try:
                        final = self.states[self.states.index(finalState.content)]
                        if final not in self.final:
                            self.final.append(final)
                    except:
                        printError("The final state: " + finalState.content + " was not declared as state.")
                        raise
                    
        if len(self.alphabet) == 0:
            raise
        
    def __findRule(self, state, symbol, findAll = False, backward = False):
        if findAll:
            rules = []
            
        for rule in self.rules:
            controlState = rule.stateTo if backward else rule.stateFrom
            if controlState == state and (symbol is None or rule.symbol == symbol):
                if findAll:
                    rules.append(rule)
                else:
                    return rule
            
        if findAll:
            return rules
        else:
            return None
    
    def __findGroup(self, groups, state):
        for group in groups:
            if state in group:
                return group
            
        return None
                    
    def minimize(self):
        # Initialize groups
        groups = []
        groups.append(StateGroup(self.final))
        tmpList = []
        for state in self.states:
            if state not in self.final:
                tmpList.append(state)
        groups.append(StateGroup(tmpList))
        
        divided = True
        while divided:
            divided = False
            for group in groups:
                groupDict = dict()
                for symbol in self.alphabet:
                    for state in group:
                        # Find rule that goes from 'state' through 'symbol'
                        rule = self.__findRule(state, symbol)
                        # Get state this rule leads to
                        outState = rule.stateTo
                        # Find group for this state
                        outGroup = self.__findGroup(groups, outState)
                        
                        if outGroup in groupDict:
                            # If group already in dictionary, add current state to it
                            groupDict[outGroup].append(state)
                        else:
                            # If group isn't in dictionary, create array under key and add current state
                            groupDict[outGroup] = [state]
                        
                    if len(groupDict) == 1:
                        # All states gained from this symbol belong to one group
                        groupDict.clear()
                    else:
                        # Split needed as gained states belong to different groups
                        # Iterate all entries and create new groups from their arrays
                        for splitGroup, states in groupDict.items():
                            groups.append(StateGroup(states))
                            
                        # Remove original group that has been splitted
                        groups.remove(group)
                        # Set flag for next iteration
                        divided = True
                        # Break current iteration to start anew
                        break
                
                if divided:
                    # Break current iteration as collection has changed
                    break
                
        # Order states inside groups
        for group in groups:
            group.states.sort()
                    
        # Build new FA to return
        # Build new states
        newStates = []
        for group in groups:
            newStates.append(ComponentElement(str(group), False))
            
        # Copy symbols
        newSymbols = []
        for symbol in self.alphabet:
            newSymbols.append(ComponentElement(symbol.name, True))

        # Build new rules
        newRules = []
        for rule in self.rules:
            stateFrom = self.__findGroup(groups, rule.stateFrom)
            symbol = rule.symbol
            stateTo = self.__findGroup(groups, rule.stateTo)
            newRules.append(ComponentElement(str(stateFrom), False))
            newRules.append(ComponentElement(symbol.name, True))
            newRules.append(ComponentElement(str(stateTo), False))
        
        # Find new starting state
        newStart = [ComponentElement(str(self.__findGroup(groups, self.start)), False)]
        
        # Build new final states
        newFinal = []
        for group in groups:
            for finalState in self.final:
                if finalState in group:
                    # Because final symbols were grouped together at first
                    # all groups containing some final state consist only
                    # of final states.
                    newFinal.append(ComponentElement(str(group), False))
                    break
            
        return FA([newStates, newSymbols, newRules, newStart, newFinal])
    
    def findUnreachable(self):
        for state in self.states:
            # Starting state is reachable even i no rules leads to it
            if state is not self.start:
                rules = self.__findRule(state, None, True, True)
                if len(rules) == 0:
                    return state
                
        return None
                
    
    def findNonFinishing(self):
        nonFinishing = []
        # Run search from final states using backward rules
        # First copy final states to stack 
        stack = self.final[:]
        visited = []
        # Now run search, placing states into visited list
        while stack:
            currentState = stack.pop()
            if currentState not in visited:
                visited.append(currentState)
            rules = self.__findRule(currentState, None, True, True)
            
            for rule in rules:
                # Don't search again visited state
                if rule.stateFrom not in visited:
                    stack.append(rule.stateFrom)
                    
        # Now iterate all states and those not in visited are non-finishing
        # as there's not combination of rules to get from that state to final one
        for state in self.states:
            if state not in visited:
                nonFinishing.append(state)
                    
        return nonFinishing
    
    def removeStates(self, states):
        for state in states:
            # Remove rules to state
            for rule in self.__findRule(state, None, True):
                self.rules.remove(rule)
                
            # Remove rules from state
            for rule in self.__findRule(state, None, True, True):
                self.rules.remove(rule)
            
            # Remove the state
            self.states.remove(state)
            
    def makeWellSpecified(self, lowercase):
        # All states must be reachable
        unreachable = self.findUnreachable()
        if unreachable is not None:
            printError("FA contains unreachable state: " + str(unreachable) + " .")
            return False
        
        # Remove all non-finishing states
        nonFinishing = self.findNonFinishing()
        self.removeStates(nonFinishing)
        
        # Create rule for symbols not used to state QFalse
        # QFalse is created only when needed
        qFalse = None
        for state in self.states:
            for symbol in self.alphabet:
                rules = self.__findRule(state, symbol, True)
                if len(rules) != 1:
                    if len(rules) == 0:
                        if qFalse is None:
                            if lowercase:
                                qFalse = State("qfalse")
                            else:
                                qFalse = State("qFALSE")
                            self.states.append(qFalse)
                        self.rules.append(Rule(state, symbol, qFalse))
                        
                    else:
                        printError("State: " + str(state) + " has multiple rules for symbol: " + str(symbol))
                        return False
    
    def verify(self):
        # All states must be reachable
        unreachable = self.findUnreachable()
        if unreachable is not None:
            printError("FA contains unreachable state: " + str(unreachable) + " .")
            return False
        
        # All states must have exactly one rule for each symbol (deterministic FA)
        for state in self.states:
            for symbol in self.alphabet:
                rules = self.__findRule(state, symbol, True)
                if len(rules) != 1:
                    if len(rules) == 0:
                        printError("State: " + str(state) + " has no rule for symbol: " + str(symbol))
                    else:
                        printError("State: " + str(state) + " has multiple rules for symbol: " + str(symbol))
                    return False
                
        # Search for non-finishing states
        nonFinishing = self.findNonFinishing()
        
        # Only one non-finishing state is allowed
        if len(nonFinishing) > 1:
            printError("FA contains more than one non-finishing state.")
            return False
                
        return True
                    
    def __str__(self):
        s = "(\n{"
        
        # Print states
        if len(self.states) > 1:
            self.states.sort()
            for state in self.states[0:-1]:
                s += str(state) + ", "
            s += str(self.states[-1])
        elif len(self.states) == 1:
            s += str(self.states[0])
        
        s += "},\n{"
        
        # Print alphabet
        if len(self.alphabet) > 1:
            self.alphabet.sort()
            for symbol in self.alphabet[0:-1]:
                s += str(symbol) + ", "
            s += str(self.alphabet[-1])
        elif len(self.alphabet) == 1:
            s += str(self.alphabet[0])
        
        s += "},\n{\n"
        
        # Print rules
        if len(self.rules) > 1:
            self.rules.sort()
            for rule in self.rules[0:-1]:
                s += str(rule) + ",\n"
            s += str(self.rules[-1]) 
        elif len(self.rules) == 1:
            s += str(self.rules[0])
            
        s += "\n},\n"
        
        # Print starting state
        if self.start is not None:
            s += str(self.start)  
        
        s += ",\n{"
        
        # Print final states
        if len(self.final) > 1:
            self.final.sort()
            for state in self.final[0:-1]:
                s += str(state) + ", "
            s += str(self.final[-1])
        elif len(self.final) == 1:
            s += str(self.final[0])
        
        s += "}\n)\n"
        
        return s
            

class ComponentElement(object):
    def __init__(self, content, isSymbol):
        self.content = content
        self.isSymbol = isSymbol

def parseInput(settings):
    content = ""
    components = [[], [], [] , [], []]
    symbolRecording = False
    fsmDefinition = False
    openedComponent = False
    lastComponentIndex = len(components) - 1
    currentComponent = 0
    apostrophe = False
    
    for line in settings.inFile:
        for char in line:
            if symbolRecording:
                # Record everything unless it's single apostrophe
                if (char == '\''):
                    if apostrophe:
                        content += '\''
                        apostrophe = False
                    else:
                        apostrophe = True
                else:
                    if apostrophe:
                        symbolRecording = False
                        components[currentComponent].append(ComponentElement(content, True))
                        content = ""
                    else:
                        content += char
                        
            if not symbolRecording:
                # Skip everything after comment symbol to next line
                if char == '#':
                    break
                
                # Skip spaces everywhere but not in rules section (needed to check -> symbol)
                if str.isspace(char) and not openedComponent:
                    continue
    
                if fsmDefinition:
                    if openedComponent:
                        if char == '\'':
                            apostrophe = False
                            symbolRecording = True
                            components[currentComponent].append(ComponentElement(content, False))
                            content = ""
                        elif char == '}':
                            openedComponent = False
                            components[currentComponent].append(ComponentElement(content, False))
                            content = ""
                        elif char == ',':
                            components[currentComponent].append(ComponentElement(content, False))
                            content = ""
                            if currentComponent == 3:
                                openedComponent = False
                                currentComponent += 1
                        else:
                            content += char
                    else:
                        if char == '{':
                            openedComponent = True
                        elif char == ')':
                            if currentComponent == lastComponentIndex:
                                # Definition is over, finish parsing file
                                fsmDefinition = False
                            else:
                                printError("Not all components specified before right brace.")
                                return None
                        elif char == ',':
                            currentComponent += 1
                            if currentComponent == 3:
                                openedComponent = True
                        else:
                            printError("Invalid symbol '" + char + "' outside of component.")
                            return None
                else:
                    if char == '(':
                        fsmDefinition = True
                    else:
                        printError("Left brace not found before definition.")
                        return None
                        
        if symbolRecording:
            # If recording symbol, add also newline character
            content += '\n'
        
    if currentComponent != lastComponentIndex:
        printError("Invalid count of components.")
        return None
        
        
    # Strip states of leading / trailing whitespaces
    # Also remove states made from empty string
    for component in components:
        remElems = []
        for elem in component:
            if not elem.isSymbol:
                elem.content = str.strip(elem.content)
                if not elem.content:
                    remElems.append(elem)
        
        for elem in remElems:
            component.remove(elem)
        
        
    # Check rules: 
    #    - number of elements must be divisible by 3
    #    - each first and third element must be state, each second a symbol
    #    - each third element must start with -> that will be removed
    #      with whitespaces following it
    if (len(components[2]) % 3 != 0):
        printError("Invalid rule, missing something in 'state symbol -> state' tuple.")
        return None
    
    for i, elem in enumerate(components[2]):
        if i % 3 == 0 and elem.isSymbol:
            printError("Invalid rule, expected state but symbol " + elem.content + " found.")
            return None
        
        if i % 3 == 1 and not elem.isSymbol:
            printError("Invalid rule, expected symbol but state " + elem.content + " found.")
            return None
        
        if i % 3 == 2:
            if elem.isSymbol:
                printError("Invalid rule, expected state but symbol " + elem.content + " found.")
                return None
            elif elem.content[0:2] != "->":
                printError("Invalid rule, related to -> symbol.")
                return None
            else:
                # Remove -> and whitespaces
                elem.content = str.lstrip(elem.content[2:])
                
    # Finally check if all states are C identifiers
    cRe = re.compile(r'^[a-zA-Z](_?[a-zA-Z0-9]+)*$', re.UNICODE)
    for component in components:
        for elem in component:
            if not elem.isSymbol:
                result = cRe.match(elem.content)
                if not result:
                    printError("Invalid name of state: " + elem.content + " .")
                    return None
                         
    
    # Component 3 - starting state - should contain only one state and nothing else
    if len(components[3]) != 1 or components[3][0].isSymbol:
        printError("Invalid specification of starting state.")
        return None
    
    # Convert all names to lower-case if case insensitive option was enabled
    if settings.caseIns:
        for component in components:
            for elem in component:
                elem.content = elem.content.lower()
    
    #for i, component in enumerate(components):
    #    settings.outFile.write("Component " + str(i) + ":\n")
    #    for elem in component:
    #        settings.outFile.write(("Symbol: " if elem.isSymbol else "State: ") + elem.content + "\n")
    #    settings.outFile.write("\n")
        
    return components

def parseArguments():
    settings = Settings()
    # Extract only parameters (leave out script name)
    for arg in sys.argv[1:]:
        splited = str.split(arg, "=");
        if len(splited) == 1:                
            if arg == "--help":
                if len(sys.argv) - 1 == 1:
                    settings.help = True
                else:
                    printError("Help parameter not used alone!")
                    return None;
            elif arg == "-f" or arg == "--find-non-finishing":
                if settings.findNonFinishing:
                    printError("-f or --find-non-finishing used more than once!")
                    return None
                elif settings.minimize:
                    printError(arg + " can't be used with -m or --minimize!")
                    return None
                else:
                    settings.findNonFinishing = True
            elif arg == "-m" or arg == "--minimize":
                if settings.minimize:
                    printError("-m or --minimize used more than once!")
                    return None
                elif settings.findNonFinishing:
                    printError(arg + " can't be used with -f or --find-non-finishing!")
                    return None
                else:
                    settings.minimize = True
            elif arg == "-i" or arg == "--case-insensitive":
                if settings.caseIns:
                    printError("-i or --case-insensitive used more than once!")
                    return None
                else:
                    settings.caseIns = True
            elif arg == "--wsfa":
                if settings.wsfa:
                    printError("--wsfa used more than once!")
                    return None
                else:
                    settings.wsfa = True
            else:
                printError("Unknown argument used!")
                return None
        elif len(splited) == 2:
            if splited[0] == "--input":
                try:
                    inFile = open(splited[1], encoding='utf-8', mode="r")
                except IOError:
                    printError(splited[1] + " couldn't be opened for reading or doesn't exist!")
                    return None
                settings.inFile = inFile
            elif splited[0] == "--output":
                try:
                    outFile = open(splited[1], encoding="utf-8", mode="w")
                except IOError:
                    printError(splited[1] + " couldn't be created or opened for writing!")
                    return None
                settings.outFile = outFile
            else:
                printError("Unknown argument used!")
                return None
        else:
            printError("Unknown argument used!")
            return None
            
    return settings            
    
settings = parseArguments()

if settings is None:
    sys.exit(1)

if settings.help:
    print("MKA: Script for converting finite automata (FA) to it's minimal form.")
    print("Input has to be well-specified FA.")
    print("Running without parameter verifies correctness and outputs")
    print("the automata in normalised form.")
    print("Parameters:")
    print(" --help                    Outputs this help text.")
    print(" --input=<filename>        The input is read from <filename> instead stdin.")
    print(" --output=<filename>       The output is saved into <filename> instead stdout.")
    print(" -f (--find-non-finishing) The output is the only non-finishing state.")
    print("                           Can't be combined with -m.")
    print(" -m (--minimize)           The output is minimal FA.")
    print("                           Can't be combined with -f.")
    print(" -i (--case-insensitive)   All names will be converted to lowercase.")
    print(" --wsfa                    Makes script accept also deterministic FA.")
    sys.exit(0)
    
components = parseInput(settings)

if components is None:
    sys.exit(60)

try:
    fa = FA(components)
except:
    sys.exit(61)
    
if not fa.verify():
    printError("FA is not well-specified")
    sys.exit(62)

if settings.findNonFinishing:
    nonFinishing = fa.findNonFinishing()
    if nonFinishing:
        settings.outFile.write(str(nonFinishing[0]))
    
else:
    if settings.minimize:
        fa = fa.minimize()
        
    settings.outFile.write(str(fa))

sys.exit(0)

